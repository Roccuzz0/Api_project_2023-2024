#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_NAME 256
#define MIN_HEAP_CAPACITY 250
#define TABLE_SIZE 250
#define DELETED_NODE (ricetta*)(0xFFFFFFFFFFFFFFFUL)
char buff[100000];


typedef struct heapNode {
    int expiry; //scadenza
    int weight; //peso
} heapNode;
typedef struct ingredienteMinHeap {
    heapNode *nodes;
    int size;
    int capacity;
} ingredienteMinHeap;
typedef struct ingredienteHashNode {
    char *nome; //nome ingrediente
    int total_weight; //quantità totale in dell'ingrediente
    ingredienteMinHeap min_heap; //riferimento ad albero per pescare quelle con il T minore
    struct ingredienteHashNode *next; // riferimento al nodo successivo
} ingredienteHashNode;
typedef struct magazzinoHashTable {
    ingredienteHashNode **cells; // crea le celle
    int size; // numero di celle nella tabella
} magazzinoHashTable;
typedef struct coda_ingredienti {
    char *nome;
    int peso; //peso necessario
    struct coda_ingredienti* next;
}coda_ingredienti;
typedef struct ricetta {
    char *nome; //nome ricetta
    coda_ingredienti *ingredienti; //riferimento al nodo successivo della lista puntata
} ricetta;
typedef struct coda_ordini{
    char* nome_ricetta;//nome ricetta completata
    int quantita;
    int tempo_richiesta;//tempo in cui vengono richiesti,mi è sembrato di capire che non dobbiamo stampare quando vengono completati
    int peso_totale;
    struct coda_ordini* next;
}coda_ordini;
typedef struct coda_risultato{
    coda_ordini* ordini_completi;
    coda_ordini* ordini_in_sospeso;
} coda_risultato;
ricetta *ricette_hash_table[TABLE_SIZE];

void print_magazzino(magazzinoHashTable* magazzino);
void init_magazzino(magazzinoHashTable* magazzino, int size);
void print_table();
void init_hash();
ricetta* cerca_ricetta(char *nome_ricetta);
void aggiungi_ricetta(char* funz);
ricetta *crea_ricetta(char* nome_ricetta, char *funz);
coda_ingredienti* inserisci_ingrediente(coda_ingredienti * head, char* nome, int peso);
coda_ingredienti * crea_ingrediente();
void elimina_ricetta(char *nome,coda_ordini *ordini_sospesi);
void rifornimento(magazzinoHashTable* magazzino, char* string,int tempo);
void insert_min_heap(ingredienteMinHeap* min_heap, heapNode node);
void trim_trailing_whitespace(char* str);
coda_ordini* inserisci_ordine_in_sospeso(coda_ordini* ordini_in_sospeso, char* string, int tempo);
coda_ordini* crea_ordine();
void stampa_coda_ordini(coda_ordini* ordini_in_sospeso);
heapNode extract_min(ingredienteMinHeap* min_heap);
coda_risultato prepara_ordine(magazzinoHashTable* magazzino, int curr_time, coda_ordini* ordini_completi, coda_ordini* ordini_in_sospeso);
void remove_expired_from_heap(ingredienteMinHeap* min_heap, int current_time);
coda_ordini* inserisci_ordine_completo(coda_ordini* head, char* nome, int quantita, int tempo, int peso);
void spedisci_ordini(coda_ordini** ordini_completi, int peso);
coda_ordini* ordina_per_peso(coda_ordini* head);
void stampa_ordini(coda_ordini* head);


int main(){
    init_hash();
    int t = 0;
    magazzinoHashTable magazzino;
    init_magazzino(&magazzino,TABLE_SIZE);
    int tempo_carretto,peso_carretto;
    coda_ordini *ordini_completi = NULL;
    coda_ordini *ordini_in_sospeso = NULL;
    if (scanf("%d %d\n", &tempo_carretto, &peso_carretto) != 2) {
        return 1; // o un altro codice di errore
    }
    do{
        //stampa_coda_ordini(ordini_in_sospeso);
        if(t % tempo_carretto==0 && t != 0){
//            printf("\nin sospeso prima di spedire:");
//            stampa_coda_ordini(ordini_in_sospeso);
//            printf("\ncompleti prima di spedire:");
//            stampa_coda_ordini(ordini_completi);
            spedisci_ordini(&ordini_completi,peso_carretto);
//            printf("\nin sospeso:");
//            stampa_coda_ordini(ordini_in_sospeso);
//            printf("\ncompleti:");
//            printf("coda ordini completi:\n");
//            stampa_coda_ordini(ordini_completi);
//            printf("siamo al t = %d\n", t);
        }
        char *result = fgets(buff, sizeof(buff), stdin);
        (void)result;  // sopprimo il warning per la variabile non utilizzata
        char* token = strtok(buff," ");//informazione sul comando da fare
        char* funz = strtok(NULL, "");//ottengo il resto della riga
        //printf("siamo al t = %d\n", t);
        if(strcmp(token,"aggiungi_ricetta")==0){
            aggiungi_ricetta(funz);
            //print_table();
        }
        else if(strcmp(token,"rimuovi_ricetta")==0){
            elimina_ricetta(funz, ordini_in_sospeso);
        }
        else if(strcmp(token,"rifornimento")==0){
            rifornimento(&magazzino,funz,t);
            coda_risultato risultato = prepara_ordine(&magazzino, t, ordini_completi, ordini_in_sospeso);
            ordini_completi = risultato.ordini_completi;
            ordini_in_sospeso = risultato.ordini_in_sospeso;
            //print_magazzino(&magazzino);
            //stampa_coda_ordini(ordini_completi);
        }
        else if(strcmp(token,"ordine")==0){
            ordini_in_sospeso = inserisci_ordine_in_sospeso(ordini_in_sospeso,funz, t);
            coda_risultato risultato = prepara_ordine(&magazzino, t, ordini_completi, ordini_in_sospeso);
            ordini_completi = risultato.ordini_completi;
            ordini_in_sospeso = risultato.ordini_in_sospeso;
//            printf("\nin sospeso:");
//            stampa_coda_ordini(ordini_in_sospeso);
//            printf("\ncompleti:");
//            stampa_coda_ordini(ordini_completi);
//            print_magazzino(&magazzino);
        }else{
            break;
        }
//        20 1000
//aggiungi_ricetta k91ztT0f80YkrVS oEnCQez0hoNCzNLlCW2Oc9wol 7 dk8BmNQdU 14 Kj8cfb01WtqXcMEJdRVIptyTa_oI 12 dk8BmNQd2ktntRxdmrWZH3E 13 dk8BmNQdSbnWhd4_Ai07QTfxGWJ 10 dk8BmNQdSRq 10
//ordine k91ztT0f80YkrVS 6
//rifornimento dk8BmNQdSbnWhd4_Ai07QTfxGWJ 170 266 Kj8cfb01WtqXcMEJdRVIptyTa_oI 1011 124 dk8BmNQdU 880 42 nO5aUROrGby67AuVt1YW66_ 22 256 oEnCQez0hoNCzNLlCW2Oc9wol 105 319 dk8BmNQdSRq 643 198 oEnCQkpjTTuYXgr8wkfJ 210 307 Kj8cfb01WCzaNmN 130 160
//ordine k91ztT0f80YkrVS 5
//ordine k91ztT0f80YkrVS 4
//ordine k91ztT0f80YkrVS 4
//ordine k91ztT0f80YkrVS 6
//rifornimento dk8BmNQdSbnWhd4_Ai07QTfxGWJ 194 278 Kj8cfb01WtqXcMEJdRVIptyTa_oI 1086 198 dk8BmNQdU 1082 33 nO5aUROrGby67AuVt1YW66_ 28 396 oEnCQez0hoNCzNLlCW2Oc9wol 130 320 dk8BmNQdSRq 800 154 oEnCQkpjTTuYXgr8wkfJ 178 369 Kj8cfb01WCzaNmN 145 156 dk8BmNQd2ktntRxdmrWZH3E 1438 181
//ordine k91ztT0f80YkrVS 10
//rifornimento Kj8cfb01WtqXcMEJdRVIptyTa_oI 833 219 nO5aUROrGJ9v0J3wvt2GpP 665 123 dk8BmNQdU 928 154 nO5aUROrGby67AuVt1YW66_ 15 287 oEnCQez0hoNCzNLlCW2Oc9wol 140 326 dk8BmNQdSRq 535 149 Kj8cfb01WCzaNmN 227 230 dk8BmNQd2ktntRxdmrWZH3E 1486 135
//ordine k91ztT0f80YkrVS 9
//ordine k91ztT0f80YkrVS 3
//ordine k91ztT0f80YkrVS 9
//ordine k91ztT0f80YkrVS 15
//rimuovi_ricetta k91ztT0f80YkrVS
//ordine k91ztT0f6kO3L 3
//ordine k91ztT0f80YkrVS 13
//ordine k91ztT0f80YkrVS 2
//ordine k91ztT0f80YkrVS 13
//rifornimento dk8BmNQdSbnWhd4_Ai07QTfxGWJ 153 310 nO5aUROrGJ9v0J3wvt2GpP 1093 142 dk8BmNQdU 1129 141 nO5aUROrGby67AuVt1YW66_ 15 306 oEnCQez0hoNCzNLlCW2Oc9wol 103 260 dk8BmNQdSRq 595 140 Kj8cfb01WCzaNmN 213 212
//ordine k91ztT0f80YkrVS 13
//ordine k91ztT0f80YkrVS 12
//aggiungi_ricetta Kj8cfb01WZAyPsH9Adt8_RLR Kj8cfb01WCzaNmN 9 oEnCQkpjTTuYXgr8wkfJ 4 dk8BmNQdU 8 Kj8cfb01WtqXcMEJdRVIptyTa_oI 3 nO5aUROrGJ9v0J3wvt2GpP 6 oEnCQez0hoNCzNLlCW2Oc9wol 8
//ordine Kj8cfb01WZAyPsH9Adt8_RLR 4
//ordine Kj8cfb01WZAyPsH9Adt8_RLR 13
//ordine k91ztT0f80YkrVS 9
//ordine Kj8cfb01WZAyPsH9Adt8_RLR 5
//ordine Kj8cfb01WZAyPsH9Adt8_RLR 11
//aggiungi_ricetta Kj8cfb01WZAyPsH9Adt8_RLR nO5aUROrGJ9v0J3wvt2GpP 39 Kj8cfb01WtqXcMEJdRVIptyTa_oI 56 nO5aUROrGby67AuVt1YW66_ 41 dk8BmNQdSbnWhd4_Ai07QTfxGWJ 50
//rimuovi_ricetta Kj8cfb01WZAyPsH9Adt8_RLR
//ordine k91ztT0f80YkrVS 7
//ordine k91ztT0f80YkrVS 7
//ordine k91ztT0f80YkrVS 11
//ordine Kj8cfb01WZAyPsH9Adt8_RLR 7
//ordine k91ztT0f80YkrVS 13
//ordine Kj8cfb01WZAyPsH9Adt8_RLR 16
//ordine k91ztT0f80YkrVS 7
//ordine Kj8cfb01WZAyPsH9Adt8_RLR 15
//aggiungi_ricetta Kj8cfb01WuJYWnhdFI dk8BmNQdU 28 nO5aUROrGby67AuVt1YW66_ 27 oEnCQkpjTTuYXgr8wkfJ 25 dk8BmNQdSbnWhd4_Ai07QTfxGWJ 34 Kj8cfb01WtqXcMEJdRVIptyTa_oI 37
//ordine Kj8cfb01WZAyPsH9Adt8_RLR 15
//ordine Kj8cfb01WZAyPsH9Adt8_RLR 12
//ordine Kj8cfb01WZAyPsH9Adt8_RLR 5
//ordine k91ztT0f80YkrVS 15
//ordine Kj8cfb01WZAyPsH9Adt8_RLR 16
//ordine k91ztT0f80YkrVS 10
//ordine Kj8cfb01WuJYWnhdFI 2
//ordine k91ztT0f80YkrVS 15
//rimuovi_ricetta Kj8cfb01WuJYWnhdFI
//ordine Kj8cfb01WuJYWnhdFI 5
//rimuovi_ricetta k91ztT0f80YkrVS
//ordine k91ztT0f80YkrVS 1
//ordine Kj8cfb01WuJYWnhdFI 3
//ordine Kj8cfb01WuJYWnhdFI 2
//ordine Kj8cfb01WZAyPsH9Adt8_RLR 26
//ordine Kj8cfb01WuJYWnhdFI 2
//ordine k91ztT0f80YkrVS 12
//rifornimento Kj8cfb01WtqXcMEJdRVIptyTa_oI 619 247 dk8BmNQdU 730 162 nO5aUROrGby67AuVt1YW66_ 17 408 oEnCQez0hoNCzNLlCW2Oc9wol 134 415 dk8BmNQdSRq 564 103 oEnCQkpjTTuYXgr8wkfJ 223 338 Kj8cfb01WCzaNmN 243 247
//ordine Kj8cfb01WZAyPsH9Adt8_RLR 10
//rifornimento nO5aUROrGJ9v0J3wvt2GpP 925 260 nO5aUROrGby67AuVt1YW66_ 19 339 oEnCQez0hoNCzNLlCW2Oc9wol 153 379 dk8BmNQdSRq 731 209 oEnCQkpjTTuYXgr8wkfJ 195 418 Kj8cfb01WCzaNmN 150 245 dk8BmNQd2ktntRxdmrWZH3E 975 245
//ordine Kj8cfb01WZAyPsH9Adt8_RLR 15
//ordine Kj8cfb01WuJYWnhdFI 4
//ordine Kj8cfb01WuJYWnhdFI 3
//rimuovi_ricetta k91ztT0f80YkrVS
//ordine k91ztT0f80YkrVS 9
//ordine Kj8cfb01WuJYWnhdFI 5
//ordine Kj8cfb01WuJYWnhdFI 2
//ordine Kj8cfb01WuJYWnhdFI 3
//ordine Kj8cfb01WZAyPsH9Adt8_RLR 23
//ordine Kj8cfb01WZAyPsH9Adt8_RLR 7
//rifornimento dk8BmNQdSbnWhd4_Ai07QTfxGWJ 210 406 Kj8cfb01WtqXcMEJdRVIptyTa_oI 1043 218 dk8BmNQdU 898 183 nO5aUROrGby67AuVt1YW66_ 15 415 oEnCQez0hoNCzNLlCW2Oc9wol 83 317 dk8BmNQdSRq 510 129 Kj8cfb01WCzaNmN 149 301 dk8BmNQd2ktntRxdmrWZH3E 1339 135
//ordine Kj8cfb01WZAyPsH9Adt8_RLR 15
//ordine Kj8cfb01WuJYWnhdFI 6
//ordine k91ztT0f80YkrVS 11
//ordine Kj8cfb01WZAyPsH9Adt8_RLR 14
//ordine k91ztT0f60Ebxaf 1
//ordine k91ztT0f80YkrVS 5
//ordine Kj8cfb01WZAyPsH9Adt8_RLR 9
//ordine Kj8cfb01WZAyPsH9Adt8_RLR 22
//ordine Kj8cfb01WuJYWnhdFI 2
//ordine Kj8cfb01WZAyPsH9Adt8_RLR 23
//aggiungi_ricetta Kj8cfb01WXL4yOL9dBrtIbqN0 nO5aUROrGJ9v0J3wvt2GpP 17 dk8BmNQdSbnWhd4_Ai07QTfxGWJ 20 oEnCQez0hoNCzNLlCW2Oc9wol 20 dk8BmNQd2ktntRxdmrWZH3E 18
//aggiungi_ricetta Kj8cfb01WuJYWnhdFI nO5aUROrGby67AuVt1YW66_ 36 dk8BmNQd2ktntRxdmrWZH3E 26 dk8BmNQdSbnWhd4_Ai07QTfxGWJ 28 oEnCQkpjTTuYXgr8wkfJ 24
//rimuovi_ricetta Kj8cfb01WXL4yOL9dBrtIbqN0
//rimuovi_ricetta Kj8cfb01WXL4yOL9dBrtIbqN0
//rifornimento dk8BmNQdU 632 88 nO5aUROrGby67AuVt1YW66_ 22 394 oEnCQez0hoNCzNLlCW2Oc9wol 80 308 oEnCQkpjTTuYXgr8wkfJ 214 463 Kj8cfb01WCzaNmN 179 261 dk8BmNQd2ktntRxdmrWZH3E 1199 260
//ordine Kj8cfb01WuJYWnhdFI 3
//ordine k91ztT0f80YkrVS 14
//ordine Kj8cfb01WXL4yOL9dBrtIbqN0 9
//ordine Kj8cfb01WuJYWnhdFI 5
//ordine Kj8cfb01WZAyPsH9Adt8_RLR 5
//rifornimento nO5aUROrGby67AuVt1YW66_ 20 449 oEnCQez0hoNCzNLlCW2Oc9wol 110 345 dk8BmNQdSRq 758 155 oEnCQkpjTTuYXgr8wkfJ 142 306 Kj8cfb01WCzaNmN 213 307
//rifornimento Kj8cfb01WtqXcMEJdRVIptyTa_oI 784 244 nO5aUROrGby67AuVt1YW66_ 24 445 oEnCQez0hoNCzNLlCW2Oc9wol 125 438 dk8BmNQdSRq 488 261 Kj8cfb01WCzaNmN 178 215 dk8BmNQd2ktntRxdmrWZH3E 936 160
//ordine Kj8cfb01WuJYWnhdFI 1
//ordine Kj8cfb01WuJYWnhdFI 4
//ordine Kj8cfb01WuJYWnhdFI 5
//ordine k91ztT0f80YkrVS 1
//aggiungi_ricetta k91ztT0fKnERew7EteSsXg4PIx2 oEnCQkpjTTuYXgr8wkfJ 29 dk8BmNQdSRq 40 dk8BmNQdU 39
//ordine k91ztT0f80YkrVS 8
//ordine Kj8cfb01WuJYWnhdFI 4
//ordine Kj8cfb01WXL4yOL9dBrtIbqN0 7
//ordine k91ztT0f80YkrVS 5
//aggiungi_ricetta nO5aUROrGLw4Nz7Fdsvs_We3pf oEnCQez0hoNCzNLlCW2Oc9wol 2 nO5aUROrGby67AuVt1YW66_ 2 dk8BmNQd2ktntRxdmrWZH3E 1 oEnCQkpjTTuYXgr8wkfJ 2 dk8BmNQdU 1
//ordine k91ztT0fKnERew7EteSsXg4PIx2 9
//ordine Kj8cfb01WZAyPsH9Adt8_RLR 7
//ordine Kj8cfb01WXL4yOL9dBrtIbqN0 1
//ordine Kj8cfb01WZAyPsH9Adt8_RLR 5
//ordine k91ztT0fKnERew7EteSsXg4PIx2 9
//ordine nO5aUROrGLw4Nz7Fdsvs_We3pf 17
//ordine Kj8cfb01WZAyPsH9Adt8_RLR 2
//ordine Kj8cfb01WXL4yOL9dBrtIbqN0 6
//rifornimento Kj8cfb01WtqXcMEJdRVIptyTa_oI 741 126 nO5aUROrGJ9v0J3wvt2GpP 1072 302 nO5aUROrGby67AuVt1YW66_ 18 406 oEnCQez0hoNCzNLlCW2Oc9wol 122 445 dk8BmNQdSRq 801 122 oEnCQkpjTTuYXgr8wkfJ 118 373 Kj8cfb01WCzaNmN 133 250
//ordine k91ztT0fKnERew7EteSsXg4PIx2 2
//ordine nO5aUROrGLw4Nz7Fdsvs_We3pf 28
//ordine k91ztT0fKnERew7EteSsXg4PIx2 6
//ordine Kj8cfb01WZAyPsH9Adt8_RLR 8
//ordine Kj8cfb01WZAyPsH9Adt8_RLR 6
//aggiungi_ricetta Kj8cfb01WuJYWnhdFI Kj8cfb01WCzaNmN 43 oEnCQez0hoNCzNLlCW2Oc9wol 31 dk8BmNQdU 28
//ordine Kj8cfb01WuJYWnhdFI 4
//rimuovi_ricetta Kj8cfb01WXL4yOL9dBrtIbqN0
//ordine k91ztT0f80YkrVS 15
//ordine Kj8cfb01WXL4yOL9dBrtIbqN0 9
//aggiungi_ricetta nO5aUROrGLw4Nz7Fdsvs_We3pf nO5aUROrGJ9v0J3wvt2GpP 8 Kj8cfb01WCzaNmN 6 dk8BmNQdSRq 20 oEnCQkpjTTuYXgr8wkfJ 14
//ordine Kj8cfb01WuJYWnhdFI 3
//ordine k91ztT0fKnERew7EteSsXg4PIx2 4
//ordine k91ztT0f80YkrVS 14
//ordine nO5aUROrGLw4Nz7Fdsvs_We3pf 8
//rifornimento Kj8cfb01WtqXcMEJdRVIptyTa_oI 957 310 nO5aUROrGJ9v0J3wvt2GpP 1080 163 nO5aUROrGby67AuVt1YW66_ 27 412 oEnCQez0hoNCzNLlCW2Oc9wol 95 341 dk8BmNQdSRq 432 140 oEnCQkpjTTuYXgr8wkfJ 181 500 Kj8cfb01WCzaNmN 195 368 dk8BmNQd2ktntRxdmrWZH3E 1206 246
//ordine Kj8cfb01WZAyPsH9Adt8_RLR 13
//ordine k91ztT0f80YkrVS 7
//ordine k91ztT0f80YkrVS 7
//ordine k91ztT0fKnERew7EteSsXg4PIx2 6
//aggiungi_ricetta Kj8cfb01WXL4yOL9dBrtIbqN0 dk8BmNQdU 33 nO5aUROrGJ9v0J3wvt2GpP 36 oEnCQkpjTTuYXgr8wkfJ 46 dk8BmNQdSRq 42 oEnCQez0hoNCzNLlCW2Oc9wol 41
//ordine Kj8cfb01WuJYWnhdFI 5
//ordine Kj8cfb01WuJYWnhdFI 5
//ordine Kj8cfb01WZAyPsH9Adt8_RLR 4
//ordine k91ztT0f80YkrVS 6
//aggiungi_ricetta nO5aUROrGLw4Nz7Fdsvs_We3pf dk8BmNQdSRq 2 dk8BmNQdSbnWhd4_Ai07QTfxGWJ 3 Kj8cfb01WtqXcMEJdRVIptyTa_oI 2
//ordine Kj8cfb01WZAyPsH9Adt8_RLR 5
//aggiungi_ricetta k91ztT0fExA9EcSzlda dk8BmNQdSRq 12 Kj8cfb01WCzaNmN 7 oEnCQkpjTTuYXgr8wkfJ 8 dk8BmNQdU 9 dk8BmNQdSbnWhd4_Ai07QTfxGWJ 8 Kj8cfb01WtqXcMEJdRVIptyTa_oI 7 oEnCQez0hoNCzNLlCW2Oc9wol 3 dk8BmNQd2ktntRxdmrWZH3E 5
//ordine k91ztT0fKnERew7EteSsXg4PIx2 2
//ordine Kj8cfb01WXL4yOL9dBrtIbqN0 5
//ordine k91ztT0fExA9EcSzlda 8
//ordine Kj8cfb01WuJYWnhdFI 4
//rimuovi_ricetta nO5aUROrGLw4Nz7Fdsvs_We3pf
//ordine Kj8cfb01WuJYWnhdFI 5
//rifornimento dk8BmNQdSbnWhd4_Ai07QTfxGWJ 121 578 Kj8cfb01WtqXcMEJdRVIptyTa_oI 1020 161 nO5aUROrGby67AuVt1YW66_ 19 468 oEnCQez0hoNCzNLlCW2Oc9wol 116 469 oEnCQkpjTTuYXgr8wkfJ 118 416 Kj8cfb01WCzaNmN 144 277
//ordine Kj8cfb01WXL4yOL9dBrtIbqN0 9
//ordine Kj8cfb01WuJYWnhdFI 6
//ordine k91ztT0f80YkrVS 5
//ordine Kj8cfb01WuJYWnhdFI 2
//ordine k91ztT0f80YkrVS 4
//ordine k91ztT0fKnERew7EteSsXg4PIx2 6
//aggiungi_ricetta dk8BmNQd7IowK oEnCQez0hoNCzNLlCW2Oc9wol 22 nO5aUROrGJ9v0J3wvt2GpP 22 dk8BmNQd2ktntRxdmrWZH3E 22 dk8BmNQdU 20
//ordine Kj8cfb01WuJYWnhdFI 3
//rimuovi_ricetta k91ztT0fKnERew7EteSsXg4PIx2
//ordine dk8BmNQd7IowK 1
//ordine k91ztT0fKnERew7EteSsXg4PIx2 1
//aggiungi_ricetta k91ztT0fp4xxat7 dk8BmNQd2ktntRxdmrWZH3E 3 Kj8cfb01WtqXcMEJdRVIptyTa_oI 6 oEnCQez0hoNCzNLlCW2Oc9wol 4 nO5aUROrGby67AuVt1YW66_ 4 dk8BmNQdSbnWhd4_Ai07QTfxGWJ 5 Kj8cfb01WCzaNmN 6
//ordine nO5aUROrGLw4Nz7Fdsvs_We3pf 48
//aggiungi_ricetta k91ztT0frpdsex1azodLSVSaaK Kj8cfb01WCzaNmN 9 nO5aUROrGJ9v0J3wvt2GpP 8 Kj8cfb01WtqXcMEJdRVIptyTa_oI 8
//aggiungi_ricetta nO5aUROrGKxyP_u9TJz02dY oEnCQkpjTTuYXgr8wkfJ 60 nO5aUROrGby67AuVt1YW66_ 58 oEnCQez0hoNCzNLlCW2Oc9wol 55
//ordine k91ztT0frpdsex1azodLSVSaaK 11
//ordine k91ztT0f80YkrVS 6
//ordine Kj8cfb01WZAyPsH9Adt8_RLR 23
//ordine Kj8cfb01WXL4yOL9dBrtIbqN0 2
//ordine Kj8cfb01WZAyPsH9Adt8_RLR 19
//ordine nO5aUROrGKxyP_u9TJz02dY 2
//ordine Kj8cfb01WuJYWnhdFI 4
//aggiungi_ricetta dk8BmNQdU95MoY oEnCQkpjTTuYXgr8wkfJ 27 Kj8cfb01WCzaNmN 25 dk8BmNQdSRq 24 Kj8cfb01WtqXcMEJdRVIptyTa_oI 28 nO5aUROrGby67AuVt1YW66_ 33
//ordine k91ztT0fExA9EcSzlda 7
//ordine k91ztT0fKnERew7EteSsXg4PIx2 6
//ordine k91ztT0f80YkrVS 8
//ordine dk8BmNQdU95MoY 7
//ordine k91ztT0fKnERew7EteSsXg4PIx2 5
//rimuovi_ricetta nO5aUROrGKxyP_u9TJz02dY
//ordine dk8BmNQdU95MoY 3
//ordine k91ztT0fKnERew7EteSsXg4PIx2 1
//ordine nO5aUROrGLw4Nz7Fdsvs_We3pf 76
//ordine Kj8cfb01WXL4yOL9dBrtIbqN0 8
//ordine dk8BmNQdU95MoY 2
//ordine k91ztT0frpdsex1azodLSVSaaK 32
//rimuovi_ricetta dk8BmNQdU95MoY
//aggiungi_ricetta nO5aUROrGEg5GOBCbD66GGjF5y Kj8cfb01WtqXcMEJdRVIptyTa_oI 34 oEnCQez0hoNCzNLlCW2Oc9wol 42 nO5aUROrGJ9v0J3wvt2GpP 44 oEnCQkpjTTuYXgr8wkfJ 32 dk8BmNQdSRq 34
//ordine nO5aUROrGKxyP_u9TJz02dY 4
//ordine nO5aUROrGEg5GOBCbD66GGjF5y 4
//ordine Kj8cfb01WXL4yOL9dBrtIbqN0 2
//rimuovi_ricetta Kj8cfb01WuJYWnhdFI
//ordine Kj8cfb01WZAyPsH9Adt8_RLR 11
//ordine Kj8cfb01WuJYWnhdFI 3
//rifornimento dk8BmNQdSbnWhd4_Ai07QTfxGWJ 191 616 Kj8cfb01WtqXcMEJdRVIptyTa_oI 717 289 nO5aUROrGJ9v0J3wvt2GpP 885 204 nO5aUROrGby67AuVt1YW66_ 14 453 oEnCQez0hoNCzNLlCW2Oc9wol 139 535 oEnCQkpjTTuYXgr8wkfJ 114 589 Kj8cfb01WCzaNmN 219 248
//ordine Kj8cfb01WuJYWnhdFI 2
//ordine nO5aUROrGEg5GOBCbD66GGjF5y 4
//ordine k91ztT0fExA9EcSzlda 5
//ordine k91ztT0f80YkrVS 13
//ordine k91ztT0frpdsex1azodLSVSaaK 35
//ordine nO5aUROrGLw4Nz7Fdsvs_We3pf 63
//ordine k91ztT0fExA9EcSzlda 8
//ordine nO5aUROrGLw4Nz7Fdsvs_We3pf 118
//ordine dk8BmNQdU95MoY 3
//ordine Kj8cfb01WXL4yOL9dBrtIbqN0 4
//ordine k91ztT0frpdsex1azodLSVSaaK 7
//rifornimento dk8BmNQdSbnWhd4_Ai07QTfxGWJ 157 617 nO5aUROrGJ9v0J3wvt2GpP 1039 321 dk8BmNQdU 820 249 nO5aUROrGby67AuVt1YW66_ 25 559 oEnCQez0hoNCzNLlCW2Oc9wol 154 483 oEnCQkpjTTuYXgr8wkfJ 135 572 Kj8cfb01WCzaNmN 175 327
//ordine Kj8cfb01WZAyPsH9Adt8_RLR 3
//rifornimento nO5aUROrGby67AuVt1YW66_ 25 452 oEnCQez0hoNCzNLlCW2Oc9wol 129 597 oEnCQkpjTTuYXgr8wkfJ 191 471 Kj8cfb01WCzaNmN 153 328 dk8BmNQd2ktntRxdmrWZH3E 848 287
//ordine dk8BmNQd7IowK 7
//ordine dk8BmNQdU95MoY 2
//ordine nO5aUROrGKxyP_u9TJz02dY 1
//ordine nO5aUROrGLw4Nz7Fdsvs_We3pf 110
//ordine k91ztT0f80YkrVS 12
//ordine k91ztT0fp4xxat7 34
//ordine nO5aUROrGKxyP_u9TJz02dY 1
//rimuovi_ricetta nO5aUROrGEg5GOBCbD66GGjF5y
//ordine k91ztT0f80YkrVS 4
//rimuovi_ricetta Kj8cfb01WuJYWnhdFI
//ordine k91ztT0fKnERew7EteSsXg4PIx2 4
//ordine Kj8cfb01WXL4yOL9dBrtIbqN0 11
//ordine Kj8cfb01WZAyPsH9Adt8_RLR 14
//ordine k91ztT0frpdsex1azodLSVSaaK 34
//ordine k91ztT0f80YkrVS 7
//rimuovi_ricetta Kj8cfb01WXL4yOL9dBrtIbqN0
//ordine k91ztT0fp4xxat7 28
//ordine k91ztT0frpdsex1azodLSVSaaK 39
//ordine nO5aUROrGKxyP_u9TJz02dY 4
//ordine k91ztT0fp4xxat7 18
//aggiungi_ricetta k91ztT0fLdnbhxkudVA dk8BmNQdSbnWhd4_Ai07QTfxGWJ 21 nO5aUROrGJ9v0J3wvt2GpP 20 Kj8cfb01WCzaNmN 24 Kj8cfb01WtqXcMEJdRVIptyTa_oI 17 dk8BmNQdU 24 nO5aUROrGby67AuVt1YW66_ 22 dk8BmNQd2ktntRxdmrWZH3E 21 dk8BmNQdSRq 27
//ordine dk8BmNQdU95MoY 1
//ordine Kj8cfb01WXL4yOL9dBrtIbqN0 6
//rimuovi_ricetta nO5aUROrGLw4Nz7Fdsvs_We3pf
//ordine Kj8cfb01WZAyPsH9Adt8_RLR 6
//rifornimento nO5aUROrGby67AuVt1YW66_ 26 653 oEnCQez0hoNCzNLlCW2Oc9wol 86 519 dk8BmNQdSRq 452 361 oEnCQkpjTTuYXgr8wkfJ 176 647 Kj8cfb01WCzaNmN 255 258 dk8BmNQd2ktntRxdmrWZH3E 1366 239
//aggiungi_ricetta oEnCQUgDHpRIFnBYEz1rk dk8BmNQdSbnWhd4_Ai07QTfxGWJ 20 dk8BmNQd2ktntRxdmrWZH3E 22 nO5aUROrGJ9v0J3wvt2GpP 22 Kj8cfb01WtqXcMEJdRVIptyTa_oI 18 dk8BmNQdU 35 Kj8cfb01WCzaNmN 23 oEnCQez0hoNCzNLlCW2Oc9wol 20 nO5aUROrGby67AuVt1YW66_ 28
//aggiungi_ricetta nO5aUROrGXlNdc dk8BmNQdSRq 24 Kj8cfb01WCzaNmN 27 dk8BmNQd2ktntRxdmrWZH3E 34 oEnCQez0hoNCzNLlCW2Oc9wol 27 Kj8cfb01WtqXcMEJdRVIptyTa_oI 37
//ordine nO5aUROrGLw4Nz7Fdsvs_We3pf 113
//rifornimento Kj8cfb01WtqXcMEJdRVIptyTa_oI 1169 247 nO5aUROrGJ9v0J3wvt2GpP 1253 255 nO5aUROrGby67AuVt1YW66_ 25 694 oEnCQez0hoNCzNLlCW2Oc9wol 112 478 oEnCQkpjTTuYXgr8wkfJ 163 629 Kj8cfb01WCzaNmN 252 452 dk8BmNQd2ktntRxdmrWZH3E 1359 284
//ordine k91ztT0fKnERew7EteSsXg4PIx2 1
//rimuovi_ricetta k91ztT0f80YkrVS
//ordine nO5aUROrGDl9bD_NTilZYwLyy 4
//ordine k91ztT0fExA9EcSzlda 2
//ordine dk8BmNQd7IowK 5
//ordine Kj8cfb01WuJYWnhdFI 5
//rimuovi_ricetta k91ztT0f80YkrVS
//rimuovi_ricetta dk8BmNQd7IowK
//ordine dk8BmNQd7IowK 10
//ordine dk8BmNQd7IowK 11
//aggiungi_ricetta oEnCQCwxkmpX6nrKF0i Kj8cfb01WtqXcMEJdRVIptyTa_oI 9 oEnCQkpjTTuYXgr8wkfJ 8 dk8BmNQdSRq 6 nO5aUROrGby67AuVt1YW66_ 8 dk8BmNQdU 4 dk8BmNQd2ktntRxdmrWZH3E 10 dk8BmNQdSbnWhd4_Ai07QTfxGWJ 14 Kj8cfb01WCzaNmN 7
//ordine k91ztT0frpdsex1azodLSVSaaK 27
//ordine dk8BmNQd7IowK 4
//ordine nO5aUROrGXlNdc 5
//ordine k91ztT0frpdsex1azodLSVSaaK 12
//rifornimento dk8BmNQdSbnWhd4_Ai07QTfxGWJ 216 557 nO5aUROrGby67AuVt1YW66_ 29 564 oEnCQez0hoNCzNLlCW2Oc9wol 83 510 oEnCQkpjTTuYXgr8wkfJ 216 456 Kj8cfb01WCzaNmN 211 282
//aggiungi_ricetta nO5aUROrGFKpqPJodXyuCcp5Ka dk8BmNQdSRq 53 Kj8cfb01WCzaNmN 51 dk8BmNQdSbnWhd4_Ai07QTfxGWJ 53
//ordine nO5aUROrGXlNdc 2
//aggiungi_ricetta nO5aUROrGlBdtlZYgSC oEnCQkpjTTuYXgr8wkfJ 24 nO5aUROrGJ9v0J3wvt2GpP 27 nO5aUROrGby67AuVt1YW66_ 33 oEnCQez0hoNCzNLlCW2Oc9wol 13 dk8BmNQdU 23 dk8BmNQd2ktntRxdmrWZH3E 21 dk8BmNQdSRq 29 dk8BmNQdSbnWhd4_Ai07QTfxGWJ 23
//rifornimento nO5aUROrGby67AuVt1YW66_ 26 750 oEnCQez0hoNCzNLlCW2Oc9wol 98 551 oEnCQkpjTTuYXgr8wkfJ 134 553 Kj8cfb01WCzaNmN 142 459 dk8BmNQd2ktntRxdmrWZH3E 875 407
//ordine oEnCQUgDHpRIFnBYEz1rk 2
//ordine nO5aUROrGLw4Nz7Fdsvs_We3pf 81
//ordine k91ztT0fp4xxat7 7
//ordine Kj8cfb01WZAyPsH9Adt8_RLR 16
//ordine k91ztT0fKnERew7EteSsXg4PIx2 6
//ordine Kj8cfb01WXL4yOL9dBrtIbqN0 1
//ordine k91ztT0f80YkrVS 6
//ordine nO5aUROrGEg5GOBCbD66GGjF5y 2
//ordine k91ztT0frpdsex1azodLSVSaaK 17
//ordine oEnCQUgDHpRIFnBYEz1rk 5
//ordine k91ztT0frpdsex1azodLSVSaaK 18
//ordine nO5aUROrGlBdtlZYgSC 3
//ordine oEnCQNTa 2
//ordine dk8BmNQd7IowK 7
//ordine nO5aUROrGXlNdc 6
//aggiungi_ricetta dk8BmNQdFMkrzxU1B oEnCQkpjTTuYXgr8wkfJ 12 dk8BmNQd2ktntRxdmrWZH3E 14 nO5aUROrGby67AuVt1YW66_ 11 Kj8cfb01WtqXcMEJdRVIptyTa_oI 7 dk8BmNQdU 11 nO5aUROrGJ9v0J3wvt2GpP 14 dk8BmNQdSRq 21
        t++;
    }while(1);
    return 0;
}

void print_table() {
    printf("Start\n");
    for (int i = 0; i < TABLE_SIZE; ++i) {
        if (ricette_hash_table[i] == NULL) {
            printf("\t%i\t---\n", i);
        } else if (ricette_hash_table[i] == DELETED_NODE) {
            printf("\t%i\t---<deleted>\n", i);
        } else {
            printf("\t%i\t%s\t", i, ricette_hash_table[i]->nome);
            printf("ingrediente : %s, quantita; %d\n",ricette_hash_table[i]->ingredienti->nome,ricette_hash_table[i]->ingredienti->peso);
            coda_ingredienti *temp = ricette_hash_table[i]->ingredienti->next;
            while(temp!=NULL) {
                printf("ingrediente : %s, quantita; %d\n", temp->nome, temp->peso);
                temp= temp->next;
            }
        }
    }
    printf("end\n");
}
void init_hash() {
    for (int i = 0; i < TABLE_SIZE; ++i) {
        ricette_hash_table[i] = NULL; //table is empty
    }
}
unsigned int hash(char *ricetta) {
    int length = strnlen(ricetta,MAX_NAME);
    unsigned int hash_value = 0;
    for (int i = 0; i < length; i++) {
        hash_value += ricetta[i];
        hash_value = (hash_value * ricetta[i]) % TABLE_SIZE;
    }
    return hash_value;
}
void init_magazzino(magazzinoHashTable* magazzino, int size) {
    magazzino->size = size;
    magazzino->cells = (ingredienteHashNode**)calloc(size, sizeof(ingredienteHashNode*));
    if (magazzino->cells == NULL) {
        perror("Errore di allocazione memoria per magazzino");
        exit(EXIT_FAILURE);
    }
}
void print_magazzino(magazzinoHashTable* magazzino) {
    printf("Stampa magazzino:\n");
    for (int i = 0; i < magazzino->size; i++) {
        ingredienteHashNode* current = magazzino->cells[i];
        if (current == NULL) {
            printf("Cella %d: vuota\n", i);
        } else {
            while (current != NULL) {
                printf("Cella %d:\n", i);
                printf("  Ingrediente: %s\n", current->nome);
                printf("  Quantita totale: %d\n", current->total_weight);

                // Stampa di debug per il min-heap
                printf("  Numero di nodi nel min-heap: %d\n", current->min_heap.size);

                for (int j = 0; j < current->min_heap.size; j++) {
                    printf("    Scadenza: %d, Peso: %d\n",
                           current->min_heap.nodes[j].expiry,
                           current->min_heap.nodes[j].weight);
                }
                current = current->next;
            }
        }
    }
}

void aggiungi_ricetta(char* funz) {
    //nome ricetta ingrediente1 quantita ing quant...
    char* nome_ricetta = strtok(funz," ");
    if(cerca_ricetta(nome_ricetta)!=NULL){
        printf("ignorato\n");
        return;
    }
    ricetta* r = crea_ricetta(nome_ricetta, funz);
    // Calcola l'indice iniziale
    int index = hash(r->nome);
    // Usa il probing per trovare una cella disponibile
    for (int i = 0; i < TABLE_SIZE; i++) {
        // Calcola il prossimo indice da provare
        int try = (index + i) % TABLE_SIZE;
        // Controlla se la cella è vuota o cancellata
        if (ricette_hash_table[try] == NULL || ricette_hash_table[try] == DELETED_NODE) {
            //print_table();
            // Inserisce la nuova ricetta nella cella disponibile
            ricette_hash_table[try] = r;
            printf("aggiunta\n");

            //print_table();
            break;
        }
    }
}
//trovare una ricetta nella tabella
ricetta *cerca_ricetta(char *nome_ricetta){
//    printf("%d\n", hash(nome_ricetta));
//    printf("%s\n",nome_ricetta);
    int index = hash(nome_ricetta);
    for (int i = 0; i < TABLE_SIZE; i++) {
        int try = (i + index) % TABLE_SIZE;
        if(ricette_hash_table[try]==NULL){
            return NULL; // not here
        }
        if(ricette_hash_table[try]==DELETED_NODE) continue;
        if (strcmp(ricette_hash_table[try]->nome, nome_ricetta)==0){
            //printf("%s\n",ricette_hash_table[try]->nome);
            return ricette_hash_table[try];
        }
    }
    return NULL;
}
ricetta *crea_ricetta(char* nome_ricetta, char *funz){
    ricetta *nuova_ricetta = (ricetta*)malloc(sizeof(ricetta));
    nuova_ricetta->nome = strdup(nome_ricetta);  // Copia il nome della ricetta
    //primo ingrediente
    coda_ingredienti *head = NULL;
    char* token = strtok(NULL, " ");
    char* nome_ingrediente = token;
    token = strtok(NULL, " ");
    int peso = atoi(token);
    head = inserisci_ingrediente(head, nome_ingrediente, peso);
    token = strtok(NULL, " ");

    //itero fino a fine ingredienti
    while(token != NULL){
        nome_ingrediente = token;
        token = strtok(NULL, " ");
        peso = atoi(token);
        head = inserisci_ingrediente(head, nome_ingrediente, peso);
        token = strtok(NULL, " ");
    }
    nuova_ricetta->ingredienti = head;
    return nuova_ricetta;
}
coda_ingredienti* inserisci_ingrediente(coda_ingredienti* head, char* nome, int peso){
    coda_ingredienti* temp;
    temp = crea_ingrediente();
    temp->nome = strdup(nome);  // Copia il nome dell'ingrediente
    temp->peso = peso;
    if(head != NULL){
        temp->next = head;
    }
    head = temp;
    return head;
}
coda_ingredienti * crea_ingrediente(){
    coda_ingredienti * temp;
    temp = (coda_ingredienti *)malloc(sizeof(coda_ingredienti));
    temp->next = NULL;
    return temp;
}

void elimina_ricetta(char *nome_ricetta, coda_ordini *ordini_sospesi) {
    char* nome = strtok(nome_ricetta," ");
    trim_trailing_whitespace(nome);
    int index = hash(nome);
    // Controlla se la ricetta è presente negli ordini sospesi
    coda_ordini *current_ordine = ordini_sospesi;
    while (current_ordine != NULL) {
        if (strcmp(nome, current_ordine->nome_ricetta) == 0) {
            printf("ordini in sospeso\n");
            return;
        }
        current_ordine = current_ordine->next;
    }
    // Cerca e cancella la ricetta dalla tabella hash
    for (int i = 0; i < TABLE_SIZE; i++) {
        int try = (i + index) % TABLE_SIZE;
        if (ricette_hash_table[try] == NULL) {
            printf("non presente\n");
            return;
        }
        if (ricette_hash_table[try] == DELETED_NODE) continue;
        if (strcmp(ricette_hash_table[try]->nome, nome) == 0) {
            ricette_hash_table[try] = DELETED_NODE;  // Marca la cella come cancellata
            printf("rimossa\n");
            return;
        }
    }
}
void rifornimento(magazzinoHashTable* magazzino, char* string, int tempo) {
    char* ingrediente = strtok(string, " ");
    while (ingrediente != NULL) {
        int peso_ingrediente = atoi(strtok(NULL, " "));
        int scadenza_ingrediente = atoi(strtok(NULL, " "));
        int index = hash(ingrediente);
        ingredienteHashNode* ingrediente_node;
        int trovato = 0;
        for (int i = 0; i < TABLE_SIZE; i++) {
            int try = (i + index) % TABLE_SIZE;
            if (magazzino->cells[try]==NULL || strcmp(magazzino->cells[try]->nome,ingrediente)==0) {
                ingrediente_node = magazzino->cells[try];
                //printf("%s %d\n",ingrediente,try);
                trovato = 1;
                break;
            }
        }
        if(trovato == 0){
            printf("ho finito gli spazi nel magazzino\n");
            return;
        }
        // Se non esiste il nodo, lo crea e setta tutto a zero
        if (ingrediente_node == NULL) {
            // Creazione del nuovo nodo se l'ingrediente non è trovato
            ingrediente_node = (ingredienteHashNode*)malloc(sizeof(ingredienteHashNode));
            ingrediente_node->nome = strdup(ingrediente);
            ingrediente_node->total_weight = 0;
            ingrediente_node->min_heap.size = 0;
            ingrediente_node->min_heap.capacity = MIN_HEAP_CAPACITY; // Capacità iniziale
            ingrediente_node->min_heap.nodes = (heapNode*)malloc(ingrediente_node->min_heap.capacity * sizeof(heapNode));
            ingrediente_node->next = magazzino->cells[index];
            magazzino->cells[index] = ingrediente_node;
        }

        // Rimuovi gli elementi scaduti prima di aggiungere nuovi elementi
        remove_expired_from_heap(&ingrediente_node->min_heap, tempo);

        // Aggiorna il peso totale
        ingrediente_node->total_weight += peso_ingrediente;

        // Inserisci il nuovo nodo nel min heap
        heapNode new_node = { .expiry = scadenza_ingrediente, .weight = peso_ingrediente };
        insert_min_heap(&ingrediente_node->min_heap, new_node);

        ingrediente = strtok(NULL, " ");
    }
    printf("rifornito\n");
}

void trim_trailing_whitespace(char* str) {
    int len = strlen(str);
    while (len > 0 && (str[len - 1] == ' ' || str[len - 1] == '\n' || str[len - 1] == '\t')) {
        str[len - 1] = '\0';
        len--;
    }
}
void swap_node(heapNode* a, heapNode* b) {
    heapNode temp = *a;
    *a = *b;
    *b = temp;
}

void min_heapify(ingredienteMinHeap* min_heap, int index) {
    int smallest = index;
    int left = 2 * index + 1;
    int right = 2 * index + 2;

    // Debug: Print the current state of the heap
//     printf("Heapifying at index %d\n", index);
//     for (int i = 0; i < min_heap->size; i++) {
//         printf("Node %d: Expiry: %d, Weight: %d\n", i, min_heap->nodes[i].expiry, min_heap->nodes[i].weight);
//     }

    // Check if the left child exists and is smaller than the current node
    if (left < min_heap->size && min_heap->nodes[left].expiry < min_heap->nodes[smallest].expiry) {
        smallest = left;
    }

    // Check if the right child exists and is smaller than the current smallest node
    if (right < min_heap->size && min_heap->nodes[right].expiry < min_heap->nodes[smallest].expiry) {
        smallest = right;
    }

    // If the smallest node is not the current node, swap and continue heapifying
    if (smallest != index) {
        swap_node(&min_heap->nodes[smallest], &min_heap->nodes[index]);

        // Recursively heapify the affected subtree
        min_heapify(min_heap, smallest);
    }
}

//funzione di inserimento del min heap
void insert_min_heap(ingredienteMinHeap* min_heap, heapNode node) {

    //printf("Inserimento nodo: Scadenza = %d, Peso = %d\n", node.expiry, node.weight);
    // Check if the heap is full; if so, double its capacity
    if (min_heap->size == min_heap->capacity) {
        min_heap->capacity *= 2;
        heapNode *new_nodes = (heapNode *)realloc(min_heap->nodes, min_heap->capacity * sizeof(heapNode));
        if (new_nodes == NULL) {
            printf("Errore di realloc durante l'espansione del min heap\n");
            return;  // Esci in caso di errore di realloc
        }
        min_heap->nodes = new_nodes;
    }

    // Add the new node at the end of the heap
    int index = min_heap->size++;
    min_heap->nodes[index] = node;

    // Heapify up: move the new node to its correct position
    while (index != 0 && min_heap->nodes[(index - 1) / 2].expiry > min_heap->nodes[index].expiry) {
        swap_node(&min_heap->nodes[index], &min_heap->nodes[(index - 1) / 2]);
        index = (index - 1) / 2;
    }
}


coda_ordini* inserisci_ordine_in_sospeso(coda_ordini* ordini_in_sospeso, char* string, int tempo) {
    // Crea una copia della stringa per evitare modifiche
    char* nome = strtok(strdup(string), " ");
    int quantita = atoi(strtok(NULL, " "));
    // Verifica se la ricetta è presente nella hash map
    int index = hash(nome);
    int ricetta_trovata = 0;
    for (int i = 0; i < TABLE_SIZE; i++) {
        int try = (i + index) % TABLE_SIZE;
        if (ricette_hash_table[try] != NULL && ricette_hash_table[try] != DELETED_NODE) {
            if (strcmp(ricette_hash_table[try]->nome, nome) == 0) {
                ricetta_trovata = 1;
                break;
            }
        }
    }
    // Stampa "Accettato" o "Rifiutato" in base alla presenza della ricetta
    if (ricetta_trovata==1) {
        printf("accettato\n");

        // Creazione del nuovo nodo per la coda degli ordini
        coda_ordini *temp = crea_ordine();
        temp->nome_ricetta = strdup(nome);  // Copia del nome della ricetta
        temp->quantita = quantita;
        temp->tempo_richiesta = tempo;
        temp->peso_totale = 0;
        temp->next = NULL;

        // Inserimento nella coda
        if (ordini_in_sospeso == NULL) {
            ordini_in_sospeso = temp;
        } else {
            coda_ordini* var = ordini_in_sospeso;
            while (var->next != NULL) {
                var = var->next;
            }
            var->next = temp;
        }
    } else {
        printf("rifiutato\n");
    }
    // Libera la memoria temporanea allocata per la stringa 'nome'
    free(nome);
    return ordini_in_sospeso;
}
coda_ordini* crea_ordine() {
    coda_ordini* temp = (coda_ordini*)malloc(sizeof(coda_ordini));
    if (temp == NULL) {
        printf("Errore nell'allocazione di memoria per l'ordine.\n");
        exit(EXIT_FAILURE);  // O altra gestione dell'errore
    }
    temp->next = NULL;
    return temp;
}
void stampa_coda_ordini(coda_ordini* ordini_in_sospeso) {
    if (ordini_in_sospeso == NULL) {
        printf("La coda degli ordini e' vuota.\n");
        return;
    }
    coda_ordini* current = ordini_in_sospeso;
    while (current != NULL) {
        printf("Ordine: %s, Quantita: %d, Tempo di richiesta: %d\n",current->nome_ricetta, current->quantita, current->tempo_richiesta);
        current = current->next;
    }
}
// Funzione aggiornata
coda_risultato prepara_ordine(magazzinoHashTable* magazzino, int curr_time, coda_ordini* ordini_completi, coda_ordini* ordini_in_sospeso) {
    coda_risultato risultato = {ordini_completi, ordini_in_sospeso };

    if (risultato.ordini_in_sospeso == NULL) {
        return risultato;
    }

    coda_ordini* curr = risultato.ordini_in_sospeso;
    coda_ordini* prec = NULL;

    while (curr != NULL) {
        char* nome_ricetta = curr->nome_ricetta;
        int quantita = curr->quantita;
        int tempo_richiesta = curr->tempo_richiesta;
        ricetta* ricetta = cerca_ricetta(nome_ricetta);

        if (!ricetta) {
            prec = curr;
            curr = curr->next;
            continue;
        }

        int ingredienti_disponibili = 1;
        coda_ingredienti* ingrediente_corrente = ricetta->ingredienti;
        // Verifica se gli ingredienti sono disponibili
        while (ingrediente_corrente != NULL) {
            int quantita_richiesta = quantita * ingrediente_corrente->peso;
            ingredienteHashNode* nodo_ingrediente = magazzino->cells[hash(ingrediente_corrente->nome)];

            if (!nodo_ingrediente) {
                ingredienti_disponibili = 0;
                break;
            }

            // Rimuovi gli ingredienti scaduti
            remove_expired_from_heap(&nodo_ingrediente->min_heap, curr_time);

            // Verifica se ci sono abbastanza ingredienti
            int peso_disponibile = 0;
            for (int i = 0; i < nodo_ingrediente->min_heap.size; i++) {
                peso_disponibile += nodo_ingrediente->min_heap.nodes[i].weight;
            }

            if (peso_disponibile < quantita_richiesta) {
                ingredienti_disponibili = 0;
                break;
            }

            ingrediente_corrente = ingrediente_corrente->next;
        }

        // Se gli ingredienti sono disponibili, rimuovi gli ingredienti e gestisci l'ordine completato
        if (ingredienti_disponibili) {
            ingrediente_corrente = ricetta->ingredienti;
            while (ingrediente_corrente != NULL) {
                int quantita_richiesta = quantita * ingrediente_corrente->peso;
                ingredienteHashNode* nodo_ingrediente = magazzino->cells[hash(ingrediente_corrente->nome)];

                while (quantita_richiesta > 0) {
                    if (nodo_ingrediente->min_heap.size == 0) {
                        break;
                    }

                    heapNode min_node = extract_min(&nodo_ingrediente->min_heap);

                    if (min_node.weight <= quantita_richiesta) {
                        quantita_richiesta -= min_node.weight;
                        nodo_ingrediente->total_weight -= min_node.weight;
                    } else {
                        min_node.weight -= quantita_richiesta;
                        nodo_ingrediente->total_weight -= quantita_richiesta;
                        quantita_richiesta = 0;

                        // Reinserisci il nodo solo se ha ancora peso
                        if (min_node.weight > 0) {
                            insert_min_heap(&nodo_ingrediente->min_heap, min_node);
                        }
                    }
                }

                ingrediente_corrente = ingrediente_corrente->next;
            }

            int peso_totale = 0;
            ingrediente_corrente = ricetta->ingredienti;
            while (ingrediente_corrente != NULL) {
                peso_totale += quantita * ingrediente_corrente->peso;
                ingrediente_corrente = ingrediente_corrente->next;
            }
            risultato.ordini_completi = inserisci_ordine_completo(risultato.ordini_completi, nome_ricetta, quantita, tempo_richiesta, peso_totale);

            // Rimozione dell'ordine corrente dalla lista
            if (prec) {
                prec->next = curr->next;
            } else {
                risultato.ordini_in_sospeso = curr->next;
            }

            coda_ordini* temp = curr;
            curr = curr->next;
            free(temp->nome_ricetta);
            free(temp);
        } else {
            prec = curr;
            curr = curr->next;
        }
    }
    return risultato;
}

heapNode extract_min(ingredienteMinHeap* min_heap) {
    if (min_heap->size <= 0) {
        heapNode empty_node = {0, 0};  // Nodo vuoto da restituire in caso di heap vuoto
        return empty_node;
    }
    if (min_heap->size == 1) {
        min_heap->size--;
        return min_heap->nodes[0];
    }
    // Prendi la radice (minimo)
    heapNode root = min_heap->nodes[0];
    // Sostituisci la radice con l'ultimo nodo e riduci la dimensione dell'heap
    min_heap->nodes[0] = min_heap->nodes[min_heap->size - 1];
    min_heap->size--;
    // Riordina l'heap
    min_heapify(min_heap, 0);

    return root;
}
void remove_expired_from_heap(ingredienteMinHeap* min_heap, int current_time) {
    // Continue removing the root as long as it is expired or has an expiry of 0
    while (min_heap->size > 0 &&
           (min_heap->nodes[0].expiry <= current_time || min_heap->nodes[0].expiry == 0)) {
        // Extract the minimum element (root) and ignore it (since it's expired)
        extract_min(min_heap);
    }
}


coda_ordini* inserisci_ordine_completo(coda_ordini* head, char* nome, int quantita, int tempo, int peso) {
    coda_ordini* temp = crea_ordine();
    temp->nome_ricetta = strdup(nome);  // Duplica il nome
    temp->quantita = quantita;
    temp->tempo_richiesta = tempo;
    temp->peso_totale = peso;
    temp->next = NULL;

    // Caso 1: La lista è vuota o l'ordine deve essere inserito all'inizio
    if (head == NULL || head->tempo_richiesta > tempo) {
        temp->next = head;
        head = temp;
    } else {
        // Caso 2: Inserimento in una posizione non iniziale
        coda_ordini* current = head;
        while (current->next != NULL && current->next->tempo_richiesta <= tempo) {
            current = current->next;
        }
        // Inserimento del nuovo ordine dopo l'elemento corrente
        temp->next = current->next;
        current->next = temp;
    }

    return head;
}

void spedisci_ordini(coda_ordini** ordini_completi, int peso_carretto) {
    if (*ordini_completi == NULL) {
        printf("camioncino vuoto\n");
        return;
    }

    // Lista di ordini da spedire
    coda_ordini* ordini_da_ordinare = NULL;

    // Lista temporanea per accumulare ordini da spedire
    coda_ordini* temp_ordinare = NULL;
    coda_ordini* prev_temp = NULL;

    coda_ordini* current = *ordini_completi;
    coda_ordini* prev = NULL;

    // Passaggio 1: Costruzione della lista di ordini da spedire
    while (current != NULL) {
        if (current->peso_totale <= peso_carretto) {
            peso_carretto -= current->peso_totale;
            //printf("Peso ordine aggiunto: %d -> Peso rimanente: %d\n", current->peso_totale, peso_carretto);


            // Rimuovi l'ordine dalla lista originale
            if (prev == NULL) {
                *ordini_completi = current->next;
            } else {
                prev->next = current->next;
            }

            // Aggiungi l'ordine alla lista temporanea
            if (temp_ordinare == NULL) {
                temp_ordinare = current;
                prev_temp = temp_ordinare;
            } else {
                prev_temp->next = current;
                prev_temp = current;
            }

            current = current->next;
            prev_temp->next = NULL; // Fine della lista temporanea
        } else {
            prev = current;
            current = current->next;
        }
    }

    // Passaggio 2: Ordinare la lista di ordini da spedire per peso decrescente e tempo di richiesta crescente
    ordini_da_ordinare = ordina_per_peso(temp_ordinare);
    // Passaggio 3: Stampare gli ordini ordinati
    //printf("Ordini da spedire:\n");
    stampa_ordini(ordini_da_ordinare);
    // Passaggio 4: Liberare la memoria degli ordini spediti
    while (ordini_da_ordinare != NULL) {
        coda_ordini* temp = ordini_da_ordinare;
        ordini_da_ordinare = ordini_da_ordinare->next;
        free(temp);
    }
}

coda_ordini* ordina_per_peso(coda_ordini* head) {
    if (head == NULL || head->next == NULL) {
        return head;
    }

    coda_ordini* sorted = NULL;

    while (head != NULL) {
        coda_ordini* current = head;
        head = head->next;

        if (sorted == NULL ||
            (sorted->peso_totale < current->peso_totale) ||
            (sorted->peso_totale == current->peso_totale && sorted->tempo_richiesta > current->tempo_richiesta)) {
            current->next = sorted;
            sorted = current;
        } else {
            coda_ordini* temp = sorted;
            while (temp->next != NULL &&
                   (temp->next->peso_totale > current->peso_totale ||
                    (temp->next->peso_totale == current->peso_totale && temp->next->tempo_richiesta <= current->tempo_richiesta))) {
                temp = temp->next;
            }
            current->next = temp->next;
            temp->next = current;
        }
    }

    return sorted;
}

void stampa_ordini(coda_ordini* head) {
    if (head == NULL) {
        printf("Nessun ordine da stampare\n");
        return;
    }

    while (head != NULL) {
        printf("%d %s %d\n", head->tempo_richiesta, head->nome_ricetta, head->quantita);
        head = head->next;
    }
}
