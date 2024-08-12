#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <stdbool.h>

#define MAX_NAME 256
#define MIN_HEAP_CAPACITY 10
#define TABLE_SIZE 50
#define DELETED_NODE (Ricetta*)(0xFFFFFFFFFFFFFFFUL)
char buff[100000];

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

typedef struct coda_ordini{
    char* nome_ricetta;//nome ricetta completata
    int tempo_richiesta;//tempo in cui vengono richiesti,mi è sembrato di capire che non dobbiamo stampare quando vengono completati
    struct coda_ordini* next;
}coda_ordini;



unsigned int hash(char* ricetta);
void swap_node(HeapNode* a, HeapNode* b);
void min_heapify(IngredienteMinHeap* min_heap, int index);
void insert_min_heap(IngredienteMinHeap* min_heap, HeapNode node);
void rifornimento(MagazzinoHashTable* magazzino, char* nome_ingrediente, int quantita, int scadenza);
void init_hash();
void print_table();
bool aggiungi_ricetta(Ricetta *r);
Ricetta* cerca_ricetta(char* nome);
Ricetta* elimina_ricetta(char* nome);
Ricetta* crea_ricetta(char* s);
Ingrediente* crea_ingrediente();
Ingrediente* inserisci_ingrediente(Ingrediente* head, char* nome, int);
HeapNode extract_min(IngredienteMinHeap* min_heap);
void remove_expired_from_heap(IngredienteMinHeap* min_heap, int current_time);
coda_ordini* crea_coda_comp();
coda_ordini* inserisci_ordine_completo(coda_ordini* head, char* nome, int tempo);
coda_ordini* rimuovi_primo_ordine_completo(coda_ordini* head);
void prepara_ordine(MagazzinoHashTable* magazzino, char* nome_ricetta, int quantità, int tempo_richiesta, coda_ordini** ordini_completati, coda_ordini** ordini_sospesi);


int main() {
    int t=0,tempo_carretto,peso_carretto;
    char* dati;
    scanf("%s",dati);
    //taglio la stringa, metto il primo elemento in tempo_carretto,metto il secondo elemento in peso_carretto
    do {
        if(t%tempo_carretto==0){
            //gestisci la rimozione dalla coda degli ordini completati e inseriscili sul carretto per poi ordinarli in modo decrescente
            //return carratto;
        }
        scanf("%s",buff);
        if(strcmp(buff,"aggiungi_ricetta")==0){
            Ricetta *r= crea_ricetta(buff);
            aggiungi_ricetta(r);
        }
        else if(strcmp(buff,"rimuovi_ricetta")==0){
            elimina_ricetta(buff);
        }
        else if(strcmp(buff,"rifornimento")==0){
            while(buff!=NULL){
                //meglio spezzare la stringa nelle rispettive funzioni, devo avere sempre i riferimenti alle teste delle code
                //inserisci_ingrediente(head,buff);
            }
        }
        else if(strcmp(buff,"ordine")==0){
            //inserisci l'ordine nelle preparazioni
        }
        t++;
    }while(strcmp(buff,NULL)==0);
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

    printf("%s %d\n", cerca_ricetta(nome_ricetta)->ingredienti->nome, cerca_ricetta(nome_ricetta)->ingredienti->peso);
    elimina_ricetta("torta");
    print_table();
    // Inizializzazione della hash table del magazzino
    MagazzinoHashTable magazzino;
    magazzino.size = TABLE_SIZE;
    magazzino.cells = (IngredienteHashNode**)calloc(TABLE_SIZE, sizeof(IngredienteHashNode*));

    // Esempio di rifornimento
    rifornimento(&magazzino, "zucchero", 100, 10);
    rifornimento(&magazzino, "farina", 200, 12);
    rifornimento(&magazzino, "zucchero", 50, 13);

    // Stampa del contenuto del magazzino per verifica
    for (int i = 0; i < TABLE_SIZE; i++) {
        IngredienteHashNode* node = magazzino.cells[i];
        while (node != NULL) {
            printf("Ingrediente: %s, Peso Totale: %d\n", node->key, node->total_weight);
            for (int j = 0; j < node->min_heap.size; j++) {
                printf("  Scadenza: %d, Peso: %d\n", node->min_heap.nodes[j].expiry, node->min_heap.nodes[j].weight);
            }
            node = node->next;
        }
    }
    //esempio rimozione ingrediente quasta forse chiamata dal main ma semplicemente quando andiamo a fare una ricetta chiamiamo questa funzione
    //su tutti gli elementi per vedere se sono scaduti e poi in base agli elementi rimasti si vede se va bene fare la ricetta o no
    //rimuovi_ingredient(&magazzino,"zucchero",11);
    // Stampa del contenuto del magazzino per verifica
    for (int i = 0; i < TABLE_SIZE; i++) {
        IngredienteHashNode* node = magazzino.cells[i];
        while (node != NULL) {
            printf("Ingrediente: %s, Peso Totale: %d\n", node->key, node->total_weight);
            for (int j = 0; j < node->min_heap.size; j++) {
                printf("  Scadenza: %d, Peso: %d\n", node->min_heap.nodes[j].expiry, node->min_heap.nodes[j].weight);
            }
            node = node->next;
        }
    }
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


unsigned int hash(char *ricetta) {
    int length = strnlen(ricetta, MAX_NAME);
    unsigned int hash_value = 0;
    for (int i = 0; i < length; ++i) {
        hash_value += ricetta[i];
        hash_value = (hash_value * ricetta[i]) % TABLE_SIZE;
    }
    return hash_value;
}
// funzione per swappare i nodi
void swap_node(HeapNode* a, HeapNode* b) {
    HeapNode temp = *a;
    *a = *b;
    *b = temp;
}

//ordinamento del min heap
void min_heapify(IngredienteMinHeap* min_heap, int index) {
    int smallest = index;
    int left = 2 * index + 1;
    int right = 2 * index + 2;

    // controllo se i figli sono dentro i limiti dell'heap
    if (left < min_heap->size && min_heap->nodes[left].expiry < min_heap->nodes[smallest].expiry) {
        smallest = left;
    }

    if (right < min_heap->size && min_heap->nodes[right].expiry < min_heap->nodes[smallest].expiry) {
        smallest = right;
    }

    // se il nodo più piccolo non è l'indice corrente, swap e continua a fare heapify
    if (smallest != index) {
        swap_node(&min_heap->nodes[smallest], &min_heap->nodes[index]);
        min_heapify(min_heap, smallest);
    }
}

//funzione di inserimento del min heap
void insert_min_heap(IngredienteMinHeap* min_heap, HeapNode node) {
    // Controlla se l'heap è pieno e, in tal caso, raddoppia la capacità
    if (min_heap->size == min_heap->capacity) {
        min_heap->capacity *= 2;
        min_heap->nodes = (HeapNode*)realloc(min_heap->nodes, min_heap->capacity * sizeof(HeapNode));
    }
    HeapNode* new_nodes = (HeapNode*)realloc(min_heap->nodes, min_heap->capacity * sizeof(HeapNode));
    if (new_nodes == NULL) {
        // Gestisci l'errore di allocazione
        perror("Errore di allocazione memoria");
        exit(EXIT_FAILURE);
    }
    min_heap->nodes = new_nodes;

    int index = min_heap->size++;// Aggiungi il nuovo nodo alla fine dell'heap
    min_heap->nodes[index] = node;
    //riordina
    while (index != 0 && min_heap->nodes[(index - 1) / 2].expiry > min_heap->nodes[index].expiry) {
        swap_node(&min_heap->nodes[index], &min_heap->nodes[(index - 1) / 2]);
        index = (index - 1) / 2;
    }
}

void rifornimento(MagazzinoHashTable* magazzino, char* nome_ingrediente, int quantita, int scadenza) {
    int index = hash(nome_ingrediente);
    IngredienteHashNode* ingrediente_node = magazzino->cells[index];

    //se non esiste il nodo lo crea e setta tutto a zero
    if (ingrediente_node == NULL) {
        // creazione del nuovo nodo se l'ingediente non è trovato
        ingrediente_node = (IngredienteHashNode*)malloc(sizeof(IngredienteHashNode));
        ingrediente_node->key = strdup(nome_ingrediente);
        ingrediente_node->total_weight = 0;
        ingrediente_node->min_heap.size = 0;
        ingrediente_node->min_heap.capacity = MIN_HEAP_CAPACITY; // Capacità iniziale
        ingrediente_node->min_heap.nodes = (HeapNode*)malloc(ingrediente_node->min_heap.capacity * sizeof(HeapNode));
        ingrediente_node->next = magazzino->cells[index];
        magazzino->cells[index] = ingrediente_node;
    }
    // aggiorna il peso totale
    ingrediente_node->total_weight += quantita;
    // inserisci il nuovo nodo nel min heap
    HeapNode new_node = { .expiry = scadenza, .weight = quantita };
    insert_min_heap(&ingrediente_node->min_heap, new_node);
}

// Funzione per estrarre il nodo con la scadenza minima dal Min-Heap
HeapNode extract_min(IngredienteMinHeap* min_heap) {
    if (min_heap->size <= 0) {
        HeapNode empty_node = {0, 0};  // Nodo vuoto da restituire in caso di heap vuoto
        return empty_node;
    }
    if (min_heap->size == 1) {
        min_heap->size--;
        return min_heap->nodes[0];
    }
    // Prendi la radice (minimo)
    HeapNode root = min_heap->nodes[0];
    // Sostituisci la radice con l'ultimo nodo e riduci la dimensione dell'heap
    min_heap->nodes[0] = min_heap->nodes[min_heap->size - 1];
    min_heap->size--;
    // Riordina l'heap
    min_heapify(min_heap, 0);

    return root;
}
//guardo solo il primo, T < curr rimuovo e vado avanti se no esco dal while, complessita o(log(n)) per rimuovere e nel caso peggiore viene fatto n volte
void remove_expired_from_heap(IngredienteMinHeap* min_heap, int current_time) {
    while (min_heap->size > 0 && min_heap->nodes[0].expiry < current_time) {
        extract_min(min_heap);  // Rimuovi l'elemento scaduto
    }
}


void prepara_ordine(MagazzinoHashTable* magazzino, char* nome_ricetta, int quantità, int tempo_richiesta, coda_ordini** ordini_completati, coda_ordini** ordini_sospesi) {
    // Trova la ricetta
    Ricetta* ricetta = cerca_ricetta(nome_ricetta);
    if (!ricetta) {
        printf("Ricetta %s non trovata.\n", nome_ricetta);
        return;
    }

    bool ingredienti_disponibili = true;  // Flag per verificare la disponibilità degli ingredienti
    // Controlla la disponibilità e rimuove ingredienti scaduti in un solo passaggio
    Ingrediente* ingrediente_corrente = ricetta->ingredienti;
    while (ingrediente_corrente!=NULL){
        int quantità_richiesta = quantità * ricetta->ingredienti->peso;
        IngredienteHashNode* nodo_ingrediente = magazzino->cells[hash(ricetta->ingredienti->nome)];
        if (!nodo_ingrediente) {
            ingredienti_disponibili = false;  // L'ingrediente non è presente
            break;
        }
        // Rimuove ingredienti scaduti e aggiorna il peso totale
        remove_expired_from_heap(&nodo_ingrediente->min_heap, tempo_richiesta);
        // Ricontrolla la disponibilità degli ingredienti
        if (nodo_ingrediente->total_weight < quantità_richiesta) {
            ingredienti_disponibili = false;  // Non c'è abbastanza quantità
            break;
        }
        ricetta->ingredienti = ricetta->ingredienti->next;
    }
    if(ingredienti_disponibili){
        while (ricetta->ingredienti->nome!=NULL){
            int quantita_richiesta = quantità * ricetta->ingredienti->peso;
            IngredienteHashNode* nodo_ingrediente = magazzino->cells[hash(ricetta->ingredienti->nome)];
            while (quantita_richiesta > 0) {
                HeapNode min_node = extract_min(&nodo_ingrediente->min_heap);
                if (min_node.weight <= quantita_richiesta) {
                    quantita_richiesta -= min_node.weight;
                    nodo_ingrediente->total_weight -= min_node.weight;
                } else {
                    min_node.weight -= quantita_richiesta;
                    nodo_ingrediente->total_weight -= quantita_richiesta;
                    quantita_richiesta = 0;
                    insert_min_heap(&nodo_ingrediente->min_heap, min_node);  // Reinserisce il nodo aggiornato
                }
            }
            ricetta->ingredienti = ricetta->ingredienti->next;
        }
        // Aggiunge l'ordine completato alla coda degli ordini completati
        *ordini_completati = inserisci_ordine_completo(*ordini_completati, nome_ricetta, tempo_richiesta);
        printf("Ordine %s completato e aggiunto agli ordini completati.\n", nome_ricetta);
    }else{
        // Aggiunge l'ordine alla coda degli ordini sospesi
        *ordini_sospesi = inserisci_ordine_completo(*ordini_sospesi, nome_ricetta, tempo_richiesta);
        printf("Ordine %s non può essere completato. Aggiunto agli ordini sospesi.\n", nome_ricetta);
    }

}








//}
//
//// funzione per rimuovere un ingrediente dalla tabella hash e dal min-heap
//void rimuovi_ingredient(MagazzinoHashTable* magazzino, char* nome_ingrediente, int current_time) {
//    int index = hash(nome_ingrediente);
//    IngredienteHashNode* ingrediente_node = magazzino->cells[index];
//    IngredienteHashNode* prev_node = NULL;
//    // trova il nodo con il nome dell'ingrediente
//    while (ingrediente_node != NULL && strcmp(ingrediente_node->key, nome_ingrediente) != 0) {
//        prev_node = ingrediente_node;
//        ingrediente_node = ingrediente_node->next;
//    }
//    // Rimuovi elementi scaduti dal min-heap
//    remove_expired_from_heap(&ingrediente_node->min_heap, current_time);
//    // Calcola il peso totale rimanente
//    int remaining_weight = 0;
//    for (int i = 0; i < ingrediente_node->min_heap.size; i++) {
//        remaining_weight += ingrediente_node->min_heap.nodes[i].weight;
//    }
//    // Aggiorna il peso totale
//    ingrediente_node->total_weight = remaining_weight;
//    // Se il peso totale è zero, elimina il nodo dalla tabella hash
//    if (ingrediente_node->total_weight == 0) {
//        if (prev_node == NULL) {
//            // Nodo da rimuovere è il primo nella lista
//            magazzino->cells[index] = ingrediente_node->next;
//        } else {
//            // Nodo da rimuovere non è il primo nella lista
//            prev_node->next = ingrediente_node->next;
//        }
//        // Libera la memoria
//        free(ingrediente_node->key);
//        free(ingrediente_node->min_heap.nodes);
//        free(ingrediente_node);
//    }
//}

//creo la tabella di hash
void init_hash() {
    for (int i = 0; i < TABLE_SIZE; ++i) {
        ricette_hash_table[i] = NULL; //table is empty
    }
}
//stampo la hash
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
//aggiungo la ricetta nella hash
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
//altro modo per rimuovere la ricetta deallocando spazio, forse leggermente meglio, bisogna stampare correttamente ancora mi sa
//void rimuovi_ricetta(char *nome) {
//    int index = hash(nome);
//    for (int i = 0; i < TABLE_SIZE; i++) {
//        int try = (i + index) % TABLE_SIZE;
//        if (ricette_hash_table[try] == NULL) {
//            // La cella è vuota, quindi non c'è nulla da rimuovere
//            return;
//        }
//        if (ricette_hash_table[try] == DELETED_NODE) {
//            // La cella è marcata come eliminata, continua a cercare
//            continue;
//        }
//        if (strcmp(ricette_hash_table[try]->nome, nome) == 0) {
//            // Trova la ricetta da rimuovere
//            Ricetta *ricetta_da_rimuovere = ricette_hash_table[try];
//
//            // Rimuove la ricetta dalla tabella hash
//            ricette_hash_table[try] = DELETED_NODE;
//
//            // Libera la memoria associata alla ricetta
//            free_ingredienti(ricetta_da_rimuovere->ingredienti);
//            free(ricetta_da_rimuovere->nome);
//            free(ricetta_da_rimuovere);
//
//            return;
//        }
//    }
//}

Ingrediente* crea_ingrediente(){
    Ingrediente* temp;
    temp = (Ingrediente*)malloc(sizeof(Ingrediente));
    temp->next = NULL;
    return temp;
}

Ingrediente* inserisci_ingrediente(Ingrediente* head, char* nome, int peso){
    Ingrediente* temp;
    temp = crea_ingrediente();
    temp->nome = nome;
    temp->peso = peso;
    if(head != NULL){
        temp->next = head;
    }
    head = temp;
    return head;
}

Ricetta *crea_ricetta(char *s){       //provare a creare una funzione di lettura nel main chiamando funzioni a seconda del primo valore
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

coda_ordini* crea_coda_comp(){
    coda_ordini* temp;
    temp = (coda_ordini*)malloc(sizeof(coda_ordini));
    temp->next = NULL;
    return temp;
}

coda_ordini* inserisci_ordine_completo(coda_ordini* head, char* nome, int tempo){
    coda_ordini * temp;
    temp=crea_coda_comp();
    temp->nome_ricetta = nome;
    temp->tempo_richiesta = tempo;
    if(head == NULL){
        head = temp;
    }else{
        coda_ordini* var;
        var = head;
        while( var->next != NULL){
            var = var->next;
        }
        var->next= temp;
    }
    return head;
}

//rimuovo il primo elemento, facendo si che venga deallocata la memoria
coda_ordini* rimuovi_primo_ordine_completo(coda_ordini* head){
    coda_ordini* temp;
    temp = head;
    head = head->next;
    coda_ordini* rimosso = crea_coda_comp();
    rimosso->nome_ricetta = temp->nome_ricetta;
    rimosso->tempo_richiesta = temp->tempo_richiesta;
    free(temp->nome_ricetta);
    free(temp);
    return rimosso;
}



//TODO
//1. Carretto -> coda ordini completi (salvando t a cui sono state completate, poi da stampare), in attesa di essere caricare sul carretto.
//creare struct per ordine che ha nome ricetta e t di completamento, che è una lista puntata (coda)
//2. Carretto -> carico degli ordini sul carretto, in ordine di peso
//3. Rivedere rimozione ingrediente nel momento in cui si cucina una ricetta (con attenzione alla scadenza)
//4. Main -> lettura stringa e chiamata funzione corretta
//5. Funzioni varie -> aggiungere stampe corrette
//6. Gestione del tempo -> incrementare variabile per ogni riga letta.
//7. Coda per ordini in attesa -> creare coda per ordini in attesa di essere completati.
//Ogni volta che arriva un ingrediente nuova, controllare se è possibile completare una ricetta in attesa.
//creerei dei valori fuori dal main che sono i nostri riferimenti tipo quando passiamo le head delle code ecc