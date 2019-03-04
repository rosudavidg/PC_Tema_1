/*
    Nume:    Rosu
    Prenume: Gabriel - David
    Grupa:   321CD
    Materia: Protocoale de comunicatii
    Titlul:  Tema 1 - mini Kermit
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "lib.h"
#include "utils.h"

#define HOST "127.0.0.1"
#define PORT 10000

int main(int argc, char** argv) {

    // Initializare variabile
    config sender_config;      // Configuratia sender-ului
    config receiver_config;    // Configuratia receiver-ului

    kermit send_kermit;        // Mesaj tip kermit pentru trimitere
    kermit recv_kermit;        // Mesaj tip kermit pentru primire

    recv_kermit.DATA = NULL;
    send_kermit.DATA = NULL;

    msg  data;                 // Auxiliar
    msg  send_msg;             // Pachet pentru trimitere
    msg* recv_msg;             // Pachet pentru primire

    unsigned char seq_no  = 0; // Numarul de secventa local

    FILE* fileptr;
    int size_left    = -1;       // Dimensiunea care a ramas de transmis din fis. actual
    int no_files     = argc - 1; // Numarul de fisiere care trebuie transmise
    int current_file = 1;        // Fisierul care trebuie transmis la un moment de timp

    // Initializare configuratie sender (conform cerintei)
    init_sender_config(&sender_config);

    // Initializare conexiune
    init(HOST, PORT);

    // Pun configuratia intr-un mesaj
    config_to_msg(sender_config, &data);
    update_kermit(&send_kermit, seq_no, TYPE_SEND_INIT, data, sender_config);
    kermit_to_msg(send_kermit, &send_msg);

    // Trimit SEND INIT
    send_message(&send_msg);
    printf("\n[S][SEQ = %d] Am trimis S.\n", seq_no);

    recv_msg = receive_message_timeout(3 * sender_config.TIME * 1000);
    if (recv_msg == NULL) {
        printf("[S][SEQ = %d] Transmisia se termina.\n", seq_no);
        return -1;
    }

    while (1) {

        // Trecerea la urmatorul mesaj
        inc_mod_64(&seq_no);

        // Mesajul primit de la receiver este convertit la kermit
        msg_to_kermit(&recv_kermit, *recv_msg);

        if (recv_kermit.TYPE == TYPE_ACK) {
            
            // Daca ultimul mesaj trimis a fost EOT, se termina conexiunea
            if (send_kermit.TYPE == TYPE_EOT) {
                printf("[S][SEQ = %d] Transmisie incheiata cu succes.\n", seq_no);
                return 0;
            }

            if (recv_kermit.DATA != NULL) {
                // Daca mesajul primit nu are campul DATA gol, atunci este
                // ceea ce trimite receiver-ul inapoi la primirea SEND-INIT

                // Copiez configuratia primita in receiver_config
                kermit_to_config(recv_kermit, &receiver_config);
            }

            if (current_file > no_files) {

                // EOT: Daca nu mai sunt fisiere de trimis, trimit EOT
                data.len = 0;
                update_kermit(&send_kermit, seq_no, TYPE_EOT, data, sender_config);
            } else {

                switch (size_left) {
                    
                    // FILE_HEADER: Incepe transmiterea unui nou fisier
                    case -1:

                        strcpy(data.payload, argv[current_file]);

                        data.len = strlen(argv[current_file]);
                        update_kermit(&send_kermit, seq_no, TYPE_FILE_HEADER, data, sender_config);

                        // Deschid fisierul si ii calculez lungimea
                        fileptr = fopen(argv[current_file], "r");

                        // Eroare daca fisierul nu exista
                        if (fileptr == NULL) {
                            printf("[S][SEQ = %d] Transmisie incheiata fara succes.\n", seq_no);
                            perror("Receive error");
                            return -1;
                        }

                        fseek(fileptr, 0, SEEK_END);
                        size_left = ftell(fileptr);
                        rewind(fileptr);
                        break;

                    // EOF: Se termina transmiterea unui fisier
                    case 0:

                        fclose(fileptr);
                        fileptr = NULL;

                        data.len = 0;
                        update_kermit(&send_kermit, seq_no, TYPE_EOF, data, sender_config);

                        current_file++;
                        size_left = -1;
                        break;

                    // DATA: Se continua transmiterea unui fisier
                    default:

                        // Citesc date
                        data.len = min(size_left, sender_config.MAXL);
                        fread(data.payload, data.len, 1, fileptr);

                        update_kermit(&send_kermit, seq_no, TYPE_DATA, data, sender_config);
                        size_left -= data.len;
                        break;
                }
            }
        } else {
            // Trimit ultimul pachet, care este deja incarcat, modificand doar seq_no
            update_kermit(&send_kermit, seq_no, send_kermit.TYPE, data, sender_config);
        }

        // Pregatesc mesajul pentru trimitere
        kermit_to_msg(send_kermit, &send_msg);
 
        unsigned char attempt = 0;

        while (1) {

            // Trimiterea mesajului catre receiver
            send_message(&send_msg);
            printf("\n[S][SEQ = %d] Am trimis %c.\n", seq_no, send_kermit.TYPE);

            recv_msg = receive_message_timeout(sender_config.TIME * 1000);

            if (recv_msg == NULL) {
                attempt++;
                printf("[S][SEQ = %d] Timeout. Attempt: %d.\n", seq_no, attempt);

                if (attempt == VALUE_ATTEMPT_MAX) {
                    printf("[S][SEQ = %d] Transmisia se incheie. Attempt = 3.\n", seq_no);
                    perror("Receive error");

                    if (fileptr != NULL) {
                        fclose(fileptr);
                    }

                    return -1;
                }
            } else {
                // Mesajul a fost primit
                break;
            }
        }
    }
}
