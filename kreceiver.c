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
#include "lib.h"
#include "utils.h"

#define HOST "127.0.0.1"
#define PORT 10001

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
    
    char* file_name = NULL;
    
    FILE* fileptr;             // Fisierul in care se scriu date

    // Initializare configuratie
    init_receiver_config(&receiver_config);

    // Initializare conexiune
    init(HOST, PORT);

    // Primire pachet SEND INIT, astept de 3 ori cate 5000ms
    recv_msg = receive_message_timeout(3 * VALUE_FIRST_TIMEOUT * 1000);

    if (recv_msg == NULL) {
        printf("[R][SEQ = %d] Nu am primit Send Init.\n", seq_no);
        printf("[R][SEQ = %d] Transmisie incheiata fara succes.\n", seq_no);
        perror("Receive error");
        return -1;
    }

    // Pentru fiecare pachet primit
    while (1) {

        // Convertire pachet primit msg -> kermit
        msg_to_kermit(&recv_kermit, *recv_msg);

        if (is_corrupt(recv_msg, recv_kermit)) {

            // Pregatirea mesajului NAK
            printf("[R][SEQ = %d] Pachet corupt.\n", seq_no);

            update_kermit(&send_kermit, seq_no, TYPE_NAK, data, receiver_config);
        } else {

            // Daca pachetul a fost primit bine, se prelucreaza informatia primita
            printf("[R][SEQ = %d] Pachet corect.\n", seq_no);

            // Mesajul pe care l-am primit in campul DATA
            get_data_from_kermit(&data, recv_kermit);
                
            // Prelucrare informatie primita
            switch (recv_kermit.TYPE) {
                    
                case TYPE_SEND_INIT:

                    // Conversie kermit -> configuratie
                    kermit_to_config(recv_kermit, &sender_config);
                    // Pun configuratia in sender_config
                    copy_config(&receiver_config, sender_config);
                    // Conversie configuratie -> msg (urmeaza a fi trimisa inapoi)
                    config_to_msg(receiver_config, &data);

                    break;

                case TYPE_FILE_HEADER:
                
                    // Deschidere fisierul pentru scriere
                    get_receiver_file_name(&file_name, data);
                    fileptr = fopen(file_name, "w");
                    free(file_name);

                    // Verificare daca exista fisierul
                    if (fileptr == NULL) {
                        printf("[R][SEQ = %d] Transmisie incheiata fara succes.\n", seq_no);
                        perror("Receive error");
                        return -1; 
                    }

                    data.len = 0;
                    break;

                case TYPE_DATA:

                    // Scriu ce am primit in campul DATA in fisier
                    fwrite(data.payload, data.len, 1, fileptr);

                    data.len = 0;
                    break;

                case TYPE_EOF:

                    // Se inchide fisierul de scriere
                    fclose(fileptr);
                    fileptr = NULL;

                    data.len = 0;
                    break;

                case TYPE_EOT:

                    // Se incheie transmisia
                    printf("\n[R][SEQ = %d] Transmisie incheiata cu succes.\n", seq_no);
                    data.len = 0;        
                    update_kermit(&send_kermit, seq_no, TYPE_ACK, data, receiver_config);
                    kermit_to_msg(send_kermit, &send_msg);
                    send_message(&send_msg);
                    return 0;

                default:
                    break;
            }
            // Pregatirea mesajului ACK
            update_kermit(&send_kermit, seq_no, TYPE_ACK, data, receiver_config);  
        }

        // Trimiterea mesajului ACK / NAK
        kermit_to_msg(send_kermit, &send_msg);
        send_message(&send_msg);

        printf("[R][SEQ = %d] Am trimis %c.\n", seq_no, send_kermit.TYPE);

        // Asteptarea urmatorului mesaj
        recv_msg = receive_message_timeout(VALUE_ATTEMPT_MAX * receiver_config.TIME * 1000);

        // Daca mesajul nu a fost primit, se incheie transmisia
        if (recv_msg == NULL) {
            printf("[R][SEQ = %d] Transmisie incheiata fara succes.\n", seq_no);
            perror("Receive error");

            if (fileptr != NULL) {
                fclose(fileptr);
            }

            return -1; 
        }

        // Trecerea la urmatorul mesaj
        inc_mod_64(&seq_no);
    }
}
