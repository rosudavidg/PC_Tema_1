```
================================================================================
Nume:    Rosu
Prenume: Gabriel - David
Grupa:   321CD
Materia: Protocoale de comunicatii
Titlul:  Tema 1 - Mini-Kermit
Data:    2 aprilie 2018
================================================================================
                                    Tema 1
                                  Mini-Kermit
================================================================================
[0] Cuprins
--------------------------------------------------------------------------------

    1. Introducere
    2. Continutul arhivei
    3. Implementare 
    4. Rulare
    5. Feedback

================================================================================
[1] Introducere
--------------------------------------------------------------------------------

    Aceasta tema reprezinta implementarea protocolului Kermit, in forma redusa,
Mini-Kermit. 
    Concret, server-ul trimite catre un receiver un numar variabil de fisiere.
Orice mesaj transmis se poate pierde sau poate ajunge corupt, pe canalul
sender -> receiver. Tema trateaza aceste erori si realizeaza transmiterea
corecta a fisierelor.

================================================================================
[2] Continului arhivei
--------------------------------------------------------------------------------
    
    +=================================================================+
    |        NUME        |               DESCRIERE                    |
    |=================================================================|
    | ksender.c          | Fisierul sursa pentru sender               |
    | kreceiver.c        | Fisierul sursa pentru receiver             |
    | utils.h            | Header folosit de ksender.c si kreceiver.c |
    | Makefile           | Makefile pentru compilare / stergere       |
    | README             | Prezentul readme                           |
    | run_experiments.sh | Scriptul pentru testarea temei             |
    | lib.h              | Header (schelet)                           |
    | link_emulator      | Realizeaza simularea conexiunii            |
    | file1.bin          | Fisier pentru testare (schelet)            |
    | file2.bin          | Fisier pentru testare (schelet)            |
    | file3.bin          | Fisier pentru testare (schelet)            |
    +=================================================================+
    
================================================================================
[3] Implementare
--------------------------------------------------------------------------------

(*) Alegerea structurilor
    
    Structurile sunt definite in utils.h.

    Structura kermit
    Contine cele 7 campuri detaliate in cerinta temei, dintre care unul, campul
DATA, are o lungime variabila aici, si este declarat sub forma unui char
pointer.
    Am ales aceasta structura pentru a fi comuna tuturor mesajelor, indiferent
de campul DATA.

    Structura config
    Contine cele 11 campuri ce completeaza definitia unui mesaj de tip kermit.
In functie de aceste campuri sunt construite mesaje (nu pe toate in aceasta
tema). Spre exemplu, campul MAXL care limiteaza lungimea campului DATA.
    Astfel, fiecare entitate sender/ receiver va avea o configuratie. Concret,
fiecare va stoca atat propria configuratie cat si pe cea a transmitatorului/
receptorului.

(*) Define-uri
    
    Define-urile se afla in utils.h.

    Cele pentru constante, prefixate cu "VALUE_", reprezinta valorile din
enuntul temei pentru diferite argumente (ex: VALUE_ATTEMPT_MAX, care este 3).

    Cele pentru tipurile de mesaje, prefixate cu "TYPE_", sunt de tip char si
semnifica codificarea tipurilor de mesaje transmise
(ex: TYPE_EOT, care este 'B'). 

(*) Functii

    * INPUT  = parametru care este util in functie
    * OUTPUT = parametru care este util dupa terminarea functiei

    +======================================================================+
    | void kermit_to_msg(kermit k, msg *message)                           |
    |----------------------------------------------------------------------|
    | Primeste un mesaj de tip kermit si il converteste la un mesaj de tip |
    |msg.                                                                  |
    | Este folosita pentru a pregati un mesaj pentru trimitere.            |
    |----------------------------------------------------------------------|
    | INPUT:  k                                                            |
    | OUTPUT: message                                                      |
    +======================================================================+

    +======================================================================+
    | void msg_to_kermit(kermit *k, msg messsage)                          |
    |----------------------------------------------------------------------|
    | Este functia inversa pentru kermit_to_msg, care converteste un mesaj |
    | de tip msg, la kermit.                                               |
    | Este folosita pentru a converti un mesaj primit.                     |
    |----------------------------------------------------------------------|
    | INPUT:  message                                                      |
    | OUTPUT: k                                                            |
    +======================================================================+

    +======================================================================+
    | void update_kermit(kermit *k_message,                                |
    |                    unsigned char seq,                                |
    |                    unsigned char type,                               |
    |                    msg data,                                         |
    |                    config config)                                    |
    |----------------------------------------------------------------------|
    | Functia primeste un mesaj de tip kermit si, in functie de ceilalti   |
    |parametri, se actualizeaza mesajul kermit.                            |
    | Este folosit pentru a schimba numarul de secventa, tipul mesajului   |
    |si campul DATA dintr-un mesaj care urmeaza a fi convertit si          |
    |apoi trimis.                                                          |
    | INPUT: k_message,                                                    |
    |        seq (numarul de secventa),                                    |
    |        type (tipul mesajului),                                       |
    |        data (continutul viitorului mesaj),                           |
    |        config (configuratia in functie de care se actualizeaza       |
    | mesajul)                                                             |
    |----------------------------------------------------------------------|
    | OUTPUT: k_message                                                    |
    +======================================================================+

    +======================================================================+
    | void config_to_msg(config config, msg *data)                         |
    |----------------------------------------------------------------------|
    | Functia converteste o configuratie de tip config la un mesaj de tip  |
    |msg.                                                                  |
    | Pentru trimiterea unui pachet de tip SEND-INIT, se va trimite        |
    |configuratia. Astfel, aceasta (config config) va fi convertita la     |
    |msg si apoi adaugata in campul DATA dintr-un kermit (vezi             |
    |urmatoarele explicatii pentru trimiterea unui mesaj).                 |
    |----------------------------------------------------------------------|
    | INPUT:  config                                                       |
    | OUTPUT: data                                                         |
    +======================================================================+

    +======================================================================+
    | void kermit_to_config(kermit k, config *cfg)                         |
    |----------------------------------------------------------------------|
    | Este aproape inversul functiei msg_to_config, doar ca extrage        |
    |direct din campul DATA al unui kermit, configuratia si o stocheaza    |
    |in cfg.                                                               |
    | Functia este utila la primirea unui pachet SEND-INIT/ primul ACK,    |
    |pentru stocarea configuratiei.                                        |
    |----------------------------------------------------------------------|
    | INPUT:  k                                                            |
    | OUTPUT: cfg                                                          |
    +======================================================================+

    +======================================================================+
    | void copy_config(config *new_cfg, config cfg)                        |
    |----------------------------------------------------------------------|
    | Primeste o configuratie si o copiaza                                 |
    |----------------------------------------------------------------------|
    | INPUT:  cfg                                                          |
    | OUTPUT: new_cfg                                                      |
    +======================================================================+

    +======================================================================+
    | void inc_mod_64(unsigned char* value)                                |
    |----------------------------------------------------------------------|
    | Primeste o valoare, o incrementeaza si aplica modulo 64.             |
    | Utila pentru incrementarea numarului de secventa.                    |
    |----------------------------------------------------------------------|
    | INPUT:  value                                                        |
    | OUTPUT: value                                                        |
    +======================================================================+

    +======================================================================+
    | void init_sender_config(config *k_config)                            |
    |----------------------------------------------------------------------|
    | Initializeaza o configuratie pentru sender, conform cerintei temei   |
    | Aceasta functiei este utilizata in ksender.c, la inceput.            |
    |----------------------------------------------------------------------|
    | OUTPUT: k_config                                                     |
    +======================================================================+

    +======================================================================+
    | void init_receiver_config(config *k_config)                          |
    |----------------------------------------------------------------------|
    | Pana primeste primul pachet SEND-INIT corect, receiverul             |
    |are nevoie de o configuratie temporara (ex: pentru timpul de timeout).|
    |----------------------------------------------------------------------|
    | OUTPUT: k_config                                                     |
    +======================================================================+

    +======================================================================+
    | bool is_corrupt(msg* recv_msg, kermit recv_kermit)                   |
    |----------------------------------------------------------------------|
    | Functia intoarce true, daca mesajul primit este corupt, false altfel |
    | Primeste mesajul primit de tip msg, caruia i se aplica functia       |
    |crc16_ccitt() si compara valoarea cu cea din campul CHECK,            |
    |din acelasi mesaj, insa in forma de kermit.                           |
    |----------------------------------------------------------------------|
    | INPUT:  recv_msg, recv_kermit                                        |
    | OUTPUT: true/false                                                   |
    +======================================================================+

    +======================================================================+
    | void get_data_from_kermit(msg* data_from_kermit, kermit recv_kermit) |
    |----------------------------------------------------------------------|
    | Copiaza doar campul DATA din recv_kermit si il intoarce in           |
    |data_from_kermit                                                      |
    |----------------------------------------------------------------------|
    | INPUT:  recv_kermit                                                  |
    | OUTPUT: data_from_kermit                                             |
    +======================================================================+

    +======================================================================+
    | void get_receiver_file_name(char** recv_file_name, msg file_name)    |
    |----------------------------------------------------------------------|
    | Primeste numele fisierului in format msg si intoarce                 |
    |un string de forma "recv_X", unde X este numele fisierului.           |
    | Functia este utila pentru a crea numele fisierului de scriere        |
    |----------------------------------------------------------------------|
    | INPUT:  file_name                                                    |
    | OUTPUT: recv_file_name                                               |
    +======================================================================+

(*) Trimiterea unui mesaj
    
    Exista: kermit send_kermit, 
            msg send_msg,
            msg data,
            unsigned char seq_no,
            config test_config
            unsigned char MSG_TYPE

    Vreau sa trimit prin campul DATA, mesajul "HELLO"

    1. Pun in campul data mesajul HELLO
        strcpy(data.payload, "HELLO");
        data.len = 5;
    
    2. Incrementez seq_no
        seq++;
    
    3. Actualizez kermit-ul in functie de aceste valori
        update_kermit(&send_kermit, seq_no, MSG_TYPE, data, test_config)

    4. Acum mesajul este pregatit in kerimit si il pot converti la msg
        kermit_to_msg(send_kermit, &send_msg)

(*) Primirea unui mesaj
    
    Mesajul primit se afla in msg* recv_msg.

    Pentru a lucra usor cu datele din el, se poate converti la kermit, astfel:

    msg_to_kermit(&recv_kermit, *recv_msg)

(*) Principiu de functionare: sender

    Initializez variabilele (ex: sender_config)
    Initializez conexiunea
    Pregatesc mesajul SEND-INIT
    Trimit mesajul SEND-INIT
    Astept de 3 * NR_SECUNDE primirea unui mesaj (ACK/ NAK)
    Daca timpul a expirat, se termina conexiunea
    Daca am primit un mesaj, atunci se intra in urmatoarea bucla:

    WHILE infinit (se termina daca conexiunea urmeaza a fi incheiata,
fie din cauza unei erori, fie in cazul finalizarii cu succes a procesului)
    - Se incrementeaza numarul de secventa
    - Se converteste ultimul mesaj primit msg -> kermit
    - Daca este NAK, se trimite ultimul pachet, incrementand doar seq
    - Daca este ACK, se continua trimiterea fisierelor in felul urmator:
        - Daca nu mai am fisiere de trimis, se trimite EOT
        - Daca mai am fisiere de trimis, dar cel curent este terminat,
        se trimite un nou fisier, FILE_HEADER
        - Daca mai am fisiere de trimis, dar am ajuns la dimensiunea ramasa
        de trimis din fisierul curent cu 0, trimit EOF
        - Daca mai am fisiere de trimis, si inca nu l-am terminat pe cel
        curent, trimit DATA.
        In plus, se verifica initial daca campul DATA primit de la receiver
        este gol. Daca nu este gol, atunci am primit configuratia receiverului
        si ea va fi stocata. Se verifica si daca ultimul mesaj primit
        este o confirmare pentru un EOT. Atunci, conexiunea se va termina
        cu succes.
    - Se pregateste mesajul pentru trimitere kermit -> msg
    - Se incearca de maxim 3 ori pentru trimiterea unui mesaj. Daca esueaza
    de 3 ori consecutiv, atunci se va termina conexiunea. Altfel, se va
    vontinua WHILE.

    return -1 - caz eroare
    return 0  - trimitere cu succes a EOT

(*) Principiu de functionare: receiver

    Initializez variabilele (inclusiv o configuratie temporara pentru receiver)
    Initializez conexiunea
    Se asteapta primirea primului mesaj (SEND-INIT), de maxim 3*TIMP secunde.
    Daca nu s-a primit, atunci se va termina conexiunea.
    Daca s-a primit, atunci se va intra in urmatoarea bucla:

    WHILE infinit, care se termina la primirea corecta a mesajului EOT sau in
caz de eroare
    - Se converteste mesajul msg -> kermit
    - Daca e corupt, se pregateste mesajul NAK
    - Daca nu e corupt, se extrage campul DATA din kermit si se prelucreaza,
in functie de tipul mesajului primit:
        SEND_INIT: se copiaza configuratia si se pregateste un ACK cu DATA
    identic cu configuratia primita.
        FILE_HEADER: se deschide un fisier corespunzator pentru scriere
        DATA: se scrie in fisierul curent deschis
        EOF: se inchide fisierul curent
        EOT: se trimite ACK si se termina cu succes conexiunea
    - Se converteste kermit -> msg pentru a fi trimis
    - Se trimite msg
    - Se asteapta de maxim 3*TIMP primirea unui mesaj de la server
    - Daca nu se primeste, atunci se termina conexiunea fara succes.
    - Altfel, se incrementeaza numarul de secventa si se repeta while-ul

    return -1 - caz eroare
    return 0  - primire succes EOT

    In plus, fisierele deja terminate sau cel putin deschise vor fi inchise
intr-un caz de eroare, pentru a se pastra datele.
    
================================================================================
[4] Rulare
--------------------------------------------------------------------------------

(*) Makefile
    
    all: build run clean

    build: compileaza sursele
    
    run: ruleaza script-ul run_experiment.sh
    
    clean: sterge fisierele obiect


(*) run_experiment.sh
    
    Este script-ul din schelet cu o modificare. Deoarece atat sender-ul, 
cat si receiver-ul afiseaza log-uri la ecran (terminal), am redirectionat
orice output catre log_send.txt, respectiv log_recv.txt.

(*) Cum?

    $ make build
    $ make run

    # + log_send.txt
    # + log_recv.txt
    # + recv_FILES
    # + fisiere obiect

    $ make clean

================================================================================
[5] Feedback
--------------------------------------------------------------------------------
    
    + O tema usoara, usor muncitoreasca, care isi atinge obiectivele.

================================================================================
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
---------------------------------------------.`` ```...-------------------------
----           ----------------------------`  /ooydd+``..-----------------------
---- +=======+ ---------..``...---://+++++.   ::+NMMM/```.----------------------
---- |SFARSIT| ------.`     ---:+ysooooooso+:.   `:-oN/ `.----------------------
---- +=======+ -----`   ohdNho-:+yooooossyyssss+:-....` `.----------------------
----           ----.   :MMMm`.-:o++ooosyysyyysyyssso+/--.:/:--------------------
-------------------.   `Md/...:+++ooosssyyhyyyyhyyyyyysysssso/------------------
-------------------.   .d:`../++++oooosyyyhyysyhyyyyyyyyyyyyysso/---------------
--------------------.     .:///+++oosssssyhhyyyhyyyyyyyyyyyyyyyyso--------------
--------------------.-.`.://////o++ooosssyyhyhyyyhyyyyyhyyyyyyysoy:-------------
-----------------------::/:////++++ooososyyyyyyyyhhyyyyyyyyyyysydh:-------------
----------------.-:----:://:////+++ooooossyyyyyhyhyyyyyyyyyyssyddy--------------
----------------.:--.---:/:::///++++++osssyyhyyhyyhyyyyyyyyyssyhh+--------------
-----------------s:-..--:::::/:///++++oossyhhyyyyyyyyyyyyysssssyy:--------------
----------------:sho-..--:::::::////++oosyyyyyyyyyyyyyyyyyyyooyyo---------------
-----------------oydy:..---::::/:///+/+oossyyyyyysyyyyhddhyoooss:---------------
-----------------/oyhs+-..-----:::///+++ossyyyyyhhdmNMNmhsoosss/----------------
------------------/oooo+/////:--:::/+++oshhddmmNMMMMNmhysoossso-----------------
-------------------:/++//::+oydmdhhdmmmNNNNNNNNNNNNmdyyoosssso:-----------------
-------------------.-://///:-:/oshhddddddmmmmmmmdhhyysosyssss/------------------
---------------------.-///+//----//+oosssyyyyyyyyyysosyyyyss/-------------------
----------------------..-:://+::--::/+://++ossyyysooyhyysss/--------------------
-----------------------.----:////:::://++/+ossssossyhyyyyo:---------------------
-----------------------.-///:-:/://///++ossssssyyhyyhhhyo:----------------------
-------------------......-:++ooo/://://+oosyhhhhhdmmdhy+------------------------
-----------------....--.--:/+/++ooosossssyhddmNNNNmdysoo+:----------------------
---------------...-/:-..--//+ossyyhddddmmNNNNNNmmhhysssss+/---------------------
----------------------..-.-:+oooyhdhddddddddddhhhyysssyyyh+/:-------------------
--------------/+/-.----------::+oyhddddmmmmmmddhyyssssyyyso--/:-----------------
------------://-..--:+yy::-::://+oosssyyyyyyyysssoossysssss+---::---------------
---------:///-..-/oymdy/::::://++oossyyyyysssssssssshmhysssso/---:--------------
-------://:-.:sdmyyhys/:::::://oyoooosssssssssssssssymNMmhsooo+:----------------
----.://::/sdNMNhssoo+:::::::+hNmoooooosssoydsoooossshdmMMNdsoooo/--------------
---.:///oshmMMmho++++::::::/hNmddyossssssssNNNsooooosyhddNMMMmsooyo:------------
--.:/+osyhmNMds+++/+/:::-:yNNddhhhossssssodmmNNyoooosshhhdNMMNNhsshy+-----------
-.:/+oshdmNNds+/////::::odNmdhhhdhssssssshdddmNNhoooosyhhhdNMNmddhyhyo----------
================================================================================
```