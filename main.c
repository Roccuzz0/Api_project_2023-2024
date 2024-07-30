#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <stdbool.h>

#define MAX_NAME 255
#define TABLE_SIZE 50
#define DELETED_NODE (Ricetta*)(0xFFFFFFFFFFFFFFFUL)

typedef struct HeapNode {
    int expiry; //scadenza
    int weight; //peso
} HeapNode;

typedef struct Ingrediente {
    HeapNode *nodes;
    int size;
} Ingrediente;

typedef struct HashNode {
    char *key; //nome ingrediente
    int total_weight; //quantità totale in dell'ingrediente
    Ingrediente min_heap; //riferimento ad albero per pescare quelle con il T minore
    struct HashNode *next; // riferimento al nodo successivo
} Magazzino;

typedef struct HashTable {
    Magazzino **cells; // crea le celle
    int size; // numero di celle nella tabella
} Dizionario_Ingredienti;

typedef struct Ricetta {
    char *ingrediente; //nome ingrediente
    int peso; //peso necessario
    struct Ricetta *next; //riferimento al nodo successivo della lista puntata
} Ricetta;

typedef struct Dizionario_ricette {
    Ricetta **cells; // celle delle ricette
    int dimension; //dimensione dell'hash table, dobbiamo renderla variabile
} Dizionario_ricette;

Ricetta *hash_table[TABLE_SIZE];

unsigned int hash(char *ricetta) {
    int length = strnlen(ricetta, MAX_NAME);
    unsigned int hash_value = 0;
    for (int i = 0; i < length; ++i) {
        hash_value += ricetta[i];
        hash_value = (hash_value * ricetta[i]) % TABLE_SIZE;
    }
    return hash_value;
}

void init_hash() {
    for (int i = 0; i < TABLE_SIZE; ++i) {
        hash_table[i] = NULL; //table is empty
    }
}

void print_table() {
    printf("Start\n");
    for (int i = 0; i < TABLE_SIZE; ++i) {
        if (hash_table[i] == NULL) {
            printf("\t%i\t---\n", i);
        } else if(hash_table[i]==DELETED_NODE) {
            printf("\t%i\t---<deleted>\n", i);
        }else{
            printf("\t%i\t%s\n", i, hash_table[i]->ingrediente);
        }
    }
    printf("end\n");
}

bool hash_insert(Ricetta *p) {
    if (p == NULL) return false;
    int index = hash(p->ingrediente);
    for (int i = 0; i < TABLE_SIZE; ++i) {
        int try = (i + index) % TABLE_SIZE;
        if(hash_table[try]==NULL){
           hash_table[try]  = DELETED_NODE;
           hash_table[try] = p;
           return true;
        }
    }
    if (hash_table[index] != NULL) {
        return false; // Cell is already occupied
    }
    return false;
}
//trovare una ricetta nella tabella
Ricetta *hash_lookup (char *nome){
    int index = hash(nome);
    for (int i = 0; i < TABLE_SIZE; ++i) {
        int try = (i + index) % TABLE_SIZE;
        if(hash_table[try]==NULL){
            return false; // not here
        }
        if(hash_table[try]==DELETED_NODE) continue;
        if (strncmp(hash_table[index]->ingrediente,nome,TABLE_SIZE)==0){
            return hash_table[try];
        }
    }
    return NULL;
}
Ricetta *hash_delete(char *nome){//cancello un elemento e ritorno l'elemento cancellato
    int index = hash(nome);
    for (int i = 0; i < TABLE_SIZE; ++i) {
        int try = (i + index) % TABLE_SIZE;
        if (hash_table[try]==NULL) return NULL;
        if (hash_table[try]==DELETED_NODE) continue;
        if (strncmp(hash_table[index]->ingrediente,nome,TABLE_SIZE)==0){
            Ricetta *tmp = hash_table[try];
            hash_table[try] = DELETED_NODE;
            return tmp;
        }
    }
    return NULL;
}

int main() {
    /*insert delete (hash, tree) main in letture, con il do while
 */
/* Mancano coda ordini da fare, coda ordini emessi
 */
    init_hash();
    print_table(); // stampa tabella vuota

    Ricetta torta = {.ingrediente = "farina", .peso = 100};
    Ricetta panna = {.ingrediente = "zucchero", .peso = 20};
    hash_insert(&torta);
    hash_insert(&panna);
    print_table();
    Ricetta *tmp = hash_lookup("farina");
    if(tmp==NULL){
        printf("non nella tabella\n");
    }else{
        printf("\nnella tabella  %s\n",tmp->ingrediente);
    }
    hash_delete("farina");
    print_table();


    /*printf("torta ==> %u\n", hash("torta"));
    printf("pane ==> %u\n", hash("pane"));
    printf("sfoglia ==> %u\n", hash("sfoglia"));
    printf("caramella ==> %u\n", hash("caramella"));
    printf("sorbetto ==> %u\n", hash("sorbetto"));
    printf("biscotto ==> %u\n", hash("biscotto"));*/

    return 0;
}
/*
 Nel main leggiamo la prima stringa e poi possiamo utilizzare una funzione che anazlizza la stringa letta e a seconda che sia:
 1)una ricetta da aggiungere/rimuovere/visualizza ingredienti;
 2)il tempo del camioncino, ovvero ogni quanti t passa;
 3)un elemento da aggiungere;
 chiama le rispettive funzioni, possiamo fare un semplice IF tanto si dovrebbe capire velocemente, se no un switch case.
 1)Per le ricette optiamo per un hashmap che le contiene tutte, quindi ci basta trovare un modo efficiente
    per creare la tabella di hash e anche una soluzione per eliminare senza problemi.
    Possimo salvarci subito il peso della ricetta, ovvero la somma di tutti gli ingredienti, in modo tale che, quando la andiamo
    a caricare sul camioncino, sappiamo subito il suo peso e non dobbiamo calcolaro.(peso dato dalla somma degli ingredienti)
 2)Potremmo non creare una funzione per questo ma salvare semplicemente in una variabile il numero di T, che corrisponde a
    ogni quanto passa il camioncino, poi ad ogni ciclo verificare se siamo nel caso %T==0 ovvero il tempo di passaggio e
    chiamare una funzione per svolgere correttamente il caricamento.
 3)Per la dispensa usiamo una hashmap, dobbiamo creare la key con due valori <prodotto,quantità>. Internamente creeremo un
    albero chiamato min-heap che ordina in base alle T ovvero avremo sulla radice il T minore e via via per ogni figlio
    da sinistra a destra in ordine crescente, complessità temporale per trovare l'elemento minimo O(1), pop e insert invece
    è O(log(n)).
 Da capire come avvengono gli ipotetici cicli di clock, perchè secondo me ogni volta che il main legge scorre un T, ma esso
 dovrebbe essere specificato nella stringa letta.Quando le ricette sono completate non ci serve più tener traccia della scadenza
 dei prodotti utilizzati ma ci serve sapere a che T sono stati completati. Quando non riusciamo a completare un ordine si crea una
 coda, dove inseriamo le ricette in attesa, ogni volta devo verificare che gli elementi siano tornati disponibili per ogni elemento
 in coda oppure metto una flag che mi permette di capire quando determinate quantità di prodotto sono arrivate e quindi
 la ricetta è cucinabile.
 Una volta completate le ricette vengono inserite in una coda in ordine di T ovvero da quella più vecchia.
 Il carretto avrà a disposizione tot grammi caricabili, specificati nel primo messaggio con i T (funzione 2).
 Esso prende in ordine le consegne fino a che non sfora i grammi (qui non so se dobbiamo verificare se si riesce a caricare qualcosa
 di più recente ma leggero oppure lasciar stare),e successivamente riordinare la lista delle consegne in ordine di peso.
 per le stampe devo ancora vedere bene ma credo basti scrivere tutti gli ordini che riesce a stampare, le ricette quando vengono aggiunte
 e rimosse e poi non saprei.
 */
