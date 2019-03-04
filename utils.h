/*
    Nume:    Rosu
    Prenume: Gabriel - David
    Grupa:   321CD
    Materia: Protocoale de comunicatii
    Titlul:  Tema 1 - mini Kermit
*/

// Fisier header pentru ksender.c si kreceiver.c

#include <stdbool.h>

// Define-uri pentru constante
#define VALUE_SOH           (unsigned char) 0x01 // Inceputul header-ului (enunt)
#define VALUE_LEN_DATA_S    (unsigned char) 11   // Lungimea campului DATA din S (enunt)
#define VALUE_ATTEMPT_MAX   (int)           3    // Numarul maxim de incercari (enunt)
#define VALUE_FIRST_TIMEOUT (int)           5    // Timpul pentru asteptarea SEND INIT

// Define-uri pentru tipurile de mesaje
#define TYPE_SEND_INIT   'S' // Configuratie
#define TYPE_FILE_HEADER 'F' // Header fisier (inceput de fisier)
#define TYPE_DATA        'D' // Date din fisier
#define TYPE_EOF         'Z' // Sfarsit de fisier
#define TYPE_EOT         'B' // Sfarsit transmisie
#define TYPE_ACK         'Y' // ACK - succes primire
#define TYPE_NAK         'N' // NAK - esec primire
#define TYPE_ERROR       'E' // Eroare

// Define-uri pentru functii
#define min(a, b) (a < b) ? a : b

// Structura pentru mini Kermit
typedef struct {
	unsigned char  SOH;      // Start of header
	unsigned char  LEN;      // Lungime totala - 2
	unsigned char  SEQ;      // Numar secventa
	unsigned char  TYPE;     // Tip de mesaj
	unsigned char* DATA;     // Campul de date
	unsigned char  CHECK[2]; // Valoare de check
	unsigned char  MARK;     // Valoare mark
} kermit; 

// Structura pentru configuratie, utila in pachetul SEND INIT
typedef struct {
	unsigned char MAXL;
	unsigned char TIME;
	unsigned char NPAD;
	unsigned char PADC;
	unsigned char EOL;
	unsigned char QCTL;
	unsigned char QBIN;
	unsigned char CHKT;
	unsigned char REPT;
	unsigned char CAPA;
	unsigned char R;
} config;

/*
	Converteste un kermit la msg, pentru
	a fi transmis drept pachet.
*/
void kermit_to_msg(kermit k, msg *message) {
	int index = 0;
	
	(*message).len = 2 + (int) k.LEN; // Lungimea payload-ului

	// Adaugare: SOH, LEN, SEQ, TYPE
	(*message).payload[index++] = k.SOH;
	(*message).payload[index++] = k.LEN;
	(*message).payload[index++] = k.SEQ;
	(*message).payload[index++] = k.TYPE;

	// Adaugare DATA
	int start_data_index = index;
	while (index < start_data_index + k.LEN - 5) {
		(*message).payload[index] = k.DATA[index - start_data_index];
		index++;
	}

	// Calculare si adaugare CHECK
    unsigned short crc = crc16_ccitt((*message).payload, (*message).len - 3);
    (*message).payload[index++] = (crc >> 8);
    (*message).payload[index++] = crc & 0xff;

    // Adaugare MARK
	(*message).payload[index++] = k.MARK;
}

/*
	Converteste un pachet primit, de tip
	msg, la tipul kermit, pentru a putea
	fi accesat mai usor
*/
void msg_to_kermit(kermit *k, msg message) {

	// Adaugare: SOH, LEN, SEQ, TYPE
	(*k).SOH  = (unsigned char) message.payload[0];
	(*k).LEN  = (unsigned char) message.payload[1];
	(*k).SEQ  = (unsigned char) message.payload[2];
	(*k).TYPE = (unsigned char) message.payload[3];

	if ((*k).DATA != NULL) {
		free((*k).DATA);
		(*k).DATA = NULL;
	}

	// Lungimea campului DATA
	if (message.len - 7 > 0) {
		(*k).DATA = malloc(sizeof(char) * (message.len - 7));		
	}

	int i;

	// Adaugare DATA
	for (i = 0; i < message.len - 7; i++) {
		(*k).DATA[i] = (unsigned char) message.payload[i + 4];
	}

	i += 4;

	// Adaugare CHECK si MARK
	(*k).CHECK[0] = (unsigned char) message.payload[i++];
	(*k).CHECK[1] = (unsigned char) message.payload[i++];
	(*k).MARK     = (unsigned char) message.payload[i];
}

/*
	In functie de configurarea sender-ului / receiver-ului, se actualizeaza un kermit.
	(In cazul acestei teme, doar pentru EOL, care oricum nu este folosit.)
	Se actualizeaza numarul de secventa, tipul mesajului si campul DATA.
*/
void update_kermit(kermit *k_message,
					unsigned char seq,
					unsigned char type,
					msg data,
					config config)
{
	(*k_message).SOH  = VALUE_SOH;
	(*k_message).LEN  = data.len + 5;
	(*k_message).SEQ  = seq;
	(*k_message).TYPE = type;

	if ((*k_message).DATA != NULL) {
		free((*k_message).DATA);
	}

	(*k_message).DATA = malloc(sizeof(char) * data.len); 

	for (int i = 0; i < data.len; i++) {
		(*k_message).DATA[i] = data.payload[i];
	}

	(*k_message).MARK = config.EOL;
}

void config_to_msg(config config, msg *data) {

	(*data).len = VALUE_LEN_DATA_S;

	(*data).payload[0]  = config.MAXL; // MAXL
	(*data).payload[1]  = config.TIME; // TIME
	(*data).payload[2]  = config.NPAD; // NPAD
	(*data).payload[3]  = config.PADC; // PADC
	(*data).payload[4]  = config.EOL;  // EOL
	(*data).payload[5]  = config.QCTL; // QCTL
	(*data).payload[6]  = config.QBIN; // QBIN
	(*data).payload[7]  = config.CHKT; // CHKT
	(*data).payload[8]  = config.REPT; // REPT
	(*data).payload[9]  = config.CAPA; // CAPA
	(*data).payload[10] = config.R;    // R
}

void kermit_to_config(kermit k, config *cfg) {
	(*cfg).MAXL = k.DATA[0];
	(*cfg).TIME = k.DATA[1];
	(*cfg).NPAD = k.DATA[2];
	(*cfg).PADC = k.DATA[3];
	(*cfg).EOL  = k.DATA[4];
	(*cfg).QCTL = k.DATA[5];
	(*cfg).QBIN = k.DATA[6];
	(*cfg).CHKT = k.DATA[7];
	(*cfg).REPT = k.DATA[8];
	(*cfg).CAPA = k.DATA[9];
	(*cfg).R    = k.DATA[10];
}

void copy_config(config *new_cfg, config cfg) {
	(*new_cfg).MAXL = cfg.MAXL;
	(*new_cfg).TIME = cfg.TIME;
	(*new_cfg).NPAD = cfg.NPAD;
	(*new_cfg).PADC = cfg.PADC;
	(*new_cfg).EOL  = cfg.EOL;
	(*new_cfg).QCTL = cfg.QCTL;
	(*new_cfg).QBIN = cfg.QBIN;
	(*new_cfg).CHKT = cfg.CHKT;
	(*new_cfg).REPT = cfg.REPT;
	(*new_cfg).CAPA = cfg.CAPA;
	(*new_cfg).R    = cfg.R;
}

/*
	Incrementeaza argumentul cu 1, modulo 64.
	value: valoarea pe care o incrementeaza
*/
void inc_mod_64(unsigned char* value) {
	*value = (*value + 1) % 64;
}

/*
	Initializez configuratia sender-ului conform cerintei.
*/
void init_sender_config(config *k_config) {
    (*k_config).MAXL = (unsigned char) 250;
    (*k_config).TIME = (unsigned char) 5;
    (*k_config).NPAD = (unsigned char) 0;
    (*k_config).PADC = (unsigned char) 0;
    (*k_config).EOL  = (unsigned char) 0x0D;
    (*k_config).QCTL = (unsigned char) 0;
    (*k_config).QBIN = (unsigned char) 0;
    (*k_config).CHKT = (unsigned char) 0;
    (*k_config).REPT = (unsigned char) 0;
    (*k_config).CAPA = (unsigned char) 0;
    (*k_config).R    = (unsigned char) 0;
}

/*
	Initializez configuratia receiver-ului, pentru a putea folosi
	o valoare initiala pentru TIME, pana cand primeste configuratia
	de la sender si o copiaza.
*/
void init_receiver_config(config *k_config) {
    (*k_config).MAXL = (unsigned char) 0;
    (*k_config).TIME = (unsigned char) VALUE_FIRST_TIMEOUT;
    (*k_config).NPAD = (unsigned char) 0;
    (*k_config).PADC = (unsigned char) 0;
    (*k_config).EOL  = (unsigned char) 0;
    (*k_config).QCTL = (unsigned char) 0;
    (*k_config).QBIN = (unsigned char) 0;
    (*k_config).CHKT = (unsigned char) 0;
    (*k_config).REPT = (unsigned char) 0;
    (*k_config).CAPA = (unsigned char) 0;
    (*k_config).R    = (unsigned char) 0;
}

/*
	Primeste un pachet, dar si un kermit (care reprezinta exact acelasi pachet).
	Se calculeaza un CRC local (crc_new) si se verifica daca este egal cu cel
	primit in pachet.
	Se intoarce true, daca mesajul a fost corupt, false altfel. 
*/
bool is_corrupt(msg* recv_msg, kermit recv_kermit) {

    unsigned short crc_new  = crc16_ccitt(recv_msg->payload, recv_msg->len - 3);
    unsigned short crc_sent = (unsigned char) recv_kermit.CHECK[0] << 8
                            | (unsigned char) recv_kermit.CHECK[1];

    return crc_new != crc_sent;
}

/*
	Copiaza doar campul DATA dintr-un mesaj de kermit.
*/
void get_data_from_kermit(msg* data_from_kermit, kermit recv_kermit) {
    (*data_from_kermit).len = recv_kermit.LEN - 5;

    for (int i = 0; i < (*data_from_kermit).len; i++) {
    	(*data_from_kermit).payload[i] = recv_kermit.DATA[i];
    }
    
    (*data_from_kermit).payload[(*data_from_kermit).len] = '\0';
}

/*
	Primeste numele fisierului care urmeaza a fi transmis.
	Se prelucreaza acesta: i se adauga in fata "recv_".
*/
void get_receiver_file_name(char** recv_file_name, msg file_name) {
	*recv_file_name = malloc(file_name.len + 6);
    sprintf(*recv_file_name, "recv_%s", file_name.payload);
}
