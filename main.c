#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <stdbool.h>

#define MAX_NAME 256
#define TABLE_SIZE 50
#define DELETED_NODE (Ricetta*)(0xFFFFFFFFFFFFFFFUL)

typedef struct HeapNode {
    int expiry; //scadenza
    int weight; //peso
} HeapNode;

typedef struct IngredienteMinHeap {
    HeapNode *nodes;
    int size;
    int capacity;
} IngredienteMinHeap;

typedef struct IngredienteHashNode {
    char *key; //nome ingrediente
    int total_weight; //quantità totale in dell'ingrediente
    IngredienteMinHeap min_heap; //riferimento ad albero per pescare quelle con il T minore
    struct HashNode *next; // riferimento al nodo successivo
} IngredienteHashNode;

typedef struct MagazzinoHashTable {
    IngredienteHashNode **cells; // crea le celle
    int size; // numero di celle nella tabella
} MagazzinoHashTable;

typedef struct Ingrediente {
    char *nome;
    int peso; //peso necessario
    struct Ingrediente* next;
} Ingrediente;

typedef struct Ricetta {
    char *nome; //nome ricetta
    Ingrediente *ingredienti; //riferimento al nodo successivo della lista puntata
} Ricetta;

// typedef struct RicetteHashTable {
//     Ricetta **cells; // celle delle ricette
//     int dimension; //dimensione dell'hash table, dobbiamo renderla variabile
// } RicetteHashTable;

Ricetta *ricette_hash_table[TABLE_SIZE];

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
        ricette_hash_table[i] = NULL; //table is empty
    }
}

void print_table() {
    printf("Start\n");
    for (int i = 0; i < TABLE_SIZE; ++i) {
        if (ricette_hash_table[i] == NULL) {
            printf("\t%i\t---\n", i);
        } else if(ricette_hash_table[i]==DELETED_NODE) {
            printf("\t%i\t---<deleted>\n", i);
        } else {
            printf("\t%i\t%s\n", i, ricette_hash_table[i]->nome);
        }
    }
    printf("end\n");
}

bool aggiungi_ricetta(Ricetta *r) {
    if (r == NULL) return false;
    int index = hash(r->nome);
    for (int i = 0; i < TABLE_SIZE; i++) {
        int try = (i + index) % TABLE_SIZE;
        if(ricette_hash_table[try]==NULL){
           ricette_hash_table[try] = DELETED_NODE;
           ricette_hash_table[try] = r;
           return true;
        }
    }
    if (ricette_hash_table[index] != NULL) {
        return false; // Cell is already occupied
    }
    return false;
}

//trovare una ricetta nella tabella
Ricetta *cerca_ricetta(char *nome){
    int index = hash(nome);
    for (int i = 0; i < TABLE_SIZE; i++) {
        int try = (i + index) % TABLE_SIZE;
        if(ricette_hash_table[try]==NULL){
            return false; // not here
        }
        if(ricette_hash_table[try]==DELETED_NODE) continue;
        if (strncmp(ricette_hash_table[index]->nome, nome, TABLE_SIZE)==0){
            return ricette_hash_table[try];
        }
    }
    return NULL;
}
Ricetta *elimina_ricetta(char *nome){//cancello un elemento e ritorno l'elemento cancellato
    int index = hash(nome);
    for (int i = 0; i < TABLE_SIZE; ++i) {
        int try = (i + index) % TABLE_SIZE;
        if (ricette_hash_table[try]==NULL) return NULL;
        if (ricette_hash_table[try]==DELETED_NODE) continue;
        if (strncmp(ricette_hash_table[index]->nome, nome, TABLE_SIZE)==0){
            Ricetta *tmp = ricette_hash_table[try];
            ricette_hash_table[try] = DELETED_NODE;
            return tmp;
        }
    }
    return NULL;
}

Ricetta* crea_ricetta(char*);

Ingrediente* inserisci_ingrediente(Ingrediente*, char*, int);

int main() {
    /*insert delete (hash, tree) main in letture, con il do while
 */
/* Mancano coda ordini da fare, coda ordini emessi
 */
    init_hash();
    print_table(); // stampa tabella vuota

    //Simula inerimento di una ricetta
    char aggiungi_ricetta_test[] = "torta farina 100 cioccolato 50 uova 20";
    char nome_ricetta[10];
    int i = 0;
    while(aggiungi_ricetta_test[i] != ' '){
        nome_ricetta[i] = aggiungi_ricetta_test[i];
        i++;
    }
    nome_ricetta[i] = '\0';
    if(cerca_ricetta(nome_ricetta) == NULL){
        Ricetta* nuova_ricetta = crea_ricetta(aggiungi_ricetta_test);
        aggiungi_ricetta(nuova_ricetta);
    }

    print_table();

    printf("%s %d", cerca_ricetta(nome_ricetta)->ingredienti->nome, cerca_ricetta(nome_ricetta)->ingredienti->peso);

    // Ricetta torta = {.nome = "farina"};
    // Ricetta panna = {.nome = "zucchero"};
    // hash_insert(&torta);
    // hash_insert(&panna);
    // print_table();
    // Ricetta *tmp = hash_lookup("farina");
    // if(tmp==NULL){
    //     printf("non nella tabella\n");
    // }else{
    //     printf("\nnella tabella  %s\n",tmp->nome);
    // }
    // hash_delete("farina");
    // print_table();


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

Ingrediente* crea_ingrediente(){
    Ingrediente* temp;
    temp = (Ingrediente*)malloc(sizeof(Ingrediente));
    temp->next = NULL;
    return temp;
}

Ingrediente* inserisci_ingrediente(Ingrediente* head, char* nome, int peso){
    Ingrediente* temp;
    Ingrediente* p;
    temp = crea_ingrediente();
    temp->nome = nome;
    temp->peso = peso;
    if(head == NULL){
        head = temp;
    }else{
        p = head;
        while(p->next != NULL){
            p = p->next; //TODO: inserendo il nodo all'inizio, si risparmierebbe in termini di tempo
        }
        p->next = temp;
    }
    return head;
}

Ricetta *crea_ricetta(char *s){
    Ricetta *nuova_ricetta = (Ricetta*)malloc(sizeof(Ricetta));
    char* token = strtok(s, " ");
    nuova_ricetta->nome = token;

    //primo ingrediente
    Ingrediente *head = NULL;
    token = strtok(NULL, " ");
    char* nome_ingrediente = token;
    token = strtok(NULL, " ");
    int peso = atoi(token);
    head = inserisci_ingrediente(head, nome_ingrediente, peso);
    nuova_ricetta->ingredienti = head;
    token = strtok(NULL, " ");

    //itero fino a fine ingredienti
    while(token != NULL){
        nome_ingrediente = token;
        token = strtok(NULL, " ");
        peso = atoi(token);
        head = inserisci_ingrediente(head, nome_ingrediente, peso);
        token = strtok(NULL, " ");
    }

    return nuova_ricetta;
}