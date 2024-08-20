#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define MAX_NAME 256
#define MIN_HEAP_CAPACITY 50
#define TABLE_SIZE 50
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
    char *key; //nome ingrediente
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
// typedef struct ricette_hash_table {
//     ricetta **cells; // celle delle ricette
//     int dimension; //dimensione dell'hash table, dobbiamo renderla variabile
// } ricette_hash_table;
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
void rifornimento(magazzinoHashTable* magazzino, char* string);
void insert_min_heap(ingredienteMinHeap* min_heap, heapNode node);
void trim_trailing_whitespace(char* str);
coda_ordini* inserisci_ordine_in_sospeso(coda_ordini* ordini_in_sospeso, char* string, int tempo);
coda_ordini* crea_ordine();
void stampa_coda_ordini(coda_ordini* ordini_in_sospeso);
heapNode extract_min(ingredienteMinHeap* min_heap);
void prepara_ordine(magazzinoHashTable* magazzino, int curr_time, coda_ordini** ordini_completi, coda_ordini** ordini_in_sospeso);
void remove_expired_from_heap(ingredienteMinHeap* min_heap, int current_time);
coda_ordini* inserisci_ordine_completo(coda_ordini* head, char* nome, int quantita, int tempo, int peso);

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
        if(t % tempo_carretto==0 && t != 0){
            //spedisci_ordini(ordini_completi,peso_carretto);
        }
        char *result = fgets(buff, sizeof(buff), stdin);
        (void)result;  // sopprimo il warning per la variabile non utilizzata
        char* token = strtok(buff," ");//informazione sul comando da fare
        char* funz = strtok(NULL, "");//ottengo il resto della riga
        if(strcmp(token,"aggiungi_ricetta")==0){
            aggiungi_ricetta(funz);
            print_table();
        }
        else if(strcmp(token,"rimuovi_ricetta")==0){
            elimina_ricetta(funz, ordini_in_sospeso);
        }
        else if(strcmp(token,"rifornimento")==0){
            rifornimento(&magazzino,funz);
            print_magazzino(&magazzino);
            prepara_ordine(&magazzino, t, &ordini_completi, &ordini_in_sospeso);
            stampa_coda_ordini(ordini_completi);
        }
        else if(strcmp(token,"ordine")==0){
            ordini_in_sospeso = inserisci_ordine_in_sospeso(ordini_in_sospeso,funz, t);
            printf("ordini in sospeso:");
            stampa_coda_ordini(ordini_in_sospeso);
            printf("ordini in completi:");
            stampa_coda_ordini(ordini_completi);
            prepara_ordine(&magazzino, t, &ordini_completi, &ordini_in_sospeso);
            printf("ordini in completi:");
            stampa_coda_ordini(ordini_completi);
        }else{
            break;
        }
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
                printf("  Ingrediente: %s\n", current->key);
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
    printf("%d\n", hash(nome_ricetta));
    printf("%s\n",nome_ricetta);
    int index = hash(nome_ricetta);
    for (int i = 0; i < TABLE_SIZE; i++) {
        int try = (i + index) % TABLE_SIZE;
        if(ricette_hash_table[try]==NULL){
            return NULL; // not here
        }
        if(ricette_hash_table[try]==DELETED_NODE) continue;
        if (strcmp(ricette_hash_table[try]->nome, nome_ricetta)==0){
            printf("%s\n",ricette_hash_table[try]->nome);
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
            printf("in sospeso\n");
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

void rifornimento(magazzinoHashTable* magazzino, char* string) {
    char* ingrediente = strtok(string, " ");
    while (ingrediente != NULL){
        int peso_ingrediente = atoi(strtok(NULL," "));
        int scadenza_ingrediente = atoi(strtok(NULL," "));
        int index = hash(ingrediente);
        ingredienteHashNode* ingrediente_node = magazzino->cells[index];
        //se non esiste il nodo lo crea e setta tutto a zero
        if (ingrediente_node == NULL) {
            // creazione del nuovo nodo se l'ingediente non è trovato
            ingrediente_node = (ingredienteHashNode*)malloc(sizeof(ingredienteHashNode));
            ingrediente_node->key = strdup(ingrediente);
            ingrediente_node->total_weight = 0;
            ingrediente_node->min_heap.size = 0;
            ingrediente_node->min_heap.capacity = MIN_HEAP_CAPACITY; // Capacità iniziale
            ingrediente_node->min_heap.nodes = (heapNode*)malloc(ingrediente_node->min_heap.capacity * sizeof(heapNode));
            ingrediente_node->next = magazzino->cells[index];
            magazzino->cells[index] = ingrediente_node;
        }
        // aggiorna il peso totale
        ingrediente_node->total_weight += peso_ingrediente;
        // inserisci il nuovo nodo nel min heap
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

//ordinamento del min heap
void min_heapify(ingredienteMinHeap* min_heap, int index) {
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
void insert_min_heap(ingredienteMinHeap* min_heap, heapNode node) {
    // Controlla se l'heap è pieno e, in tal caso, raddoppia la capacità
    if (min_heap->size == min_heap->capacity) {
        min_heap->capacity *= 2;
        heapNode* new_nodes = (heapNode*)realloc(min_heap->nodes, min_heap->capacity * sizeof(heapNode));
        if (new_nodes == NULL) {
            exit(EXIT_FAILURE);
        }
        min_heap->nodes = new_nodes;
    }
    // Aggiungi il nuovo nodo alla fine dell'heap
    int index = min_heap->size++;
    min_heap->nodes[index] = node;
    // Riordina l'heap verso l'alto
    while (index != 0 && min_heap->nodes[(index - 1) / 2].expiry > min_heap->nodes[index].expiry) {
        swap_node(&min_heap->nodes[index], &min_heap->nodes[(index - 1) / 2]);
        index = (index - 1) / 2;
    }
}coda_ordini* inserisci_ordine_in_sospeso(coda_ordini* ordini_in_sospeso, char* string, int tempo) {
    // Crea una copia della stringa per evitare modifiche
    char* nome = strtok(strdup(string), " ");
    int quantita = atoi(strtok(NULL, " "));
    // Verifica se la ricetta è presente nella hash map
    int index = hash(nome);
    bool ricetta_trovata = false;
    for (int i = 0; i < TABLE_SIZE; i++) {
        int try = (i + index) % TABLE_SIZE;
        if (ricette_hash_table[try] != NULL && ricette_hash_table[try] != DELETED_NODE) {
            if (strcmp(ricette_hash_table[try]->nome, nome) == 0) {
                ricetta_trovata = true;
                break;
            }
        }
    }
    // Stampa "Accettato" o "Rifiutato" in base alla presenza della ricetta
    if (ricetta_trovata) {
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
coda_ordini* crea_ordine(){
    coda_ordini* temp;
    temp = (coda_ordini*)malloc(sizeof(coda_ordini));
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
        printf("Ordine: %s, Quantita: %d, Tempo di richiesta: %d\n",
               current->nome_ricetta, current->quantita, current->tempo_richiesta);
        current = current->next;
    }
}
void prepara_ordine(magazzinoHashTable* magazzino, int curr_time, coda_ordini** ordini_completi, coda_ordini** ordini_in_sospeso) {
    if (*ordini_in_sospeso == NULL) {
        return;
    }

    coda_ordini* curr = *ordini_in_sospeso;
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

        bool ingredienti_disponibili = true;
        coda_ingredienti* ingrediente_corrente = ricetta->ingredienti;

        while (ingrediente_corrente != NULL) {
            int quantita_richiesta = quantita * ingrediente_corrente->peso;
            ingredienteHashNode* nodo_ingrediente = magazzino->cells[hash(ingrediente_corrente->nome)];

            if (!nodo_ingrediente) {
                ingredienti_disponibili = false;
                break;
            }

            remove_expired_from_heap(&nodo_ingrediente->min_heap, curr_time);

            if (nodo_ingrediente->total_weight < quantita_richiesta) {
                ingredienti_disponibili = false;
                break;
            }

            ingrediente_corrente = ingrediente_corrente->next;
        }

        if (ingredienti_disponibili) {
            int peso_totale = 0;
            ingrediente_corrente = ricetta->ingredienti;

            while (ingrediente_corrente != NULL) {
                int quantita_richiesta = quantita * ingrediente_corrente->peso;
                peso_totale += quantita_richiesta;
                ingredienteHashNode* nodo_ingrediente = magazzino->cells[hash(ingrediente_corrente->nome)];

                while (quantita_richiesta > 0) {
                    heapNode min_node = extract_min(&nodo_ingrediente->min_heap);
                    if (min_node.weight <= quantita_richiesta) {
                        quantita_richiesta -= min_node.weight;
                        nodo_ingrediente->total_weight -= min_node.weight;
                    } else {
                        min_node.weight -= quantita_richiesta;
                        nodo_ingrediente->total_weight -= quantita_richiesta;
                        quantita_richiesta = 0;
                        insert_min_heap(&nodo_ingrediente->min_heap, min_node);
                    }
                }

                ingrediente_corrente = ingrediente_corrente->next;
            }

            *ordini_completi = inserisci_ordine_completo(*ordini_completi, nome_ricetta, quantita, tempo_richiesta, peso_totale);

            if (prec) {
                prec->next = curr->next;
            } else {
                *ordini_in_sospeso = curr->next;
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
}

// Funzione per estrarre il nodo con la scadenza minima dal Min-Heap
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
//guardo solo il primo, T < curr rimuovo e vado avanti se no esco dal while, complessita o(log(n)) per rimuovere e nel caso peggiore viene fatto n volte
void remove_expired_from_heap(ingredienteMinHeap* min_heap, int current_time) {
    while (min_heap->size > 0 && min_heap->nodes[0].expiry < current_time) {
        extract_min(min_heap);  // Rimuovi l'elemento scaduto
    }
}
coda_ordini* inserisci_ordine_completo(coda_ordini* head, char* nome, int quantita, int tempo, int peso) {
    coda_ordini* temp = crea_ordine();
    temp->nome_ricetta = strdup(nome);  // Duplica il nome
    temp->quantita = quantita;
    temp->tempo_richiesta = tempo;
    temp->peso_totale = peso;
    temp->next = NULL;
    if (head == NULL) {
        head = temp;
    } else {
        coda_ordini* var = head;
        while (var->next != NULL) {
            var = var->next;
        }
        var->next = temp;
    }
    return head;
}


//unsigned int hash(char* ricetta);
//void swap_node(HeapNode* a, HeapNode* b);
//void min_heapify(IngredienteMinHeap* min_heap, int index);
//void insert_min_heap(IngredienteMinHeap* min_heap, HeapNode node);
//void rifornimento(MagazzinoHashTable* magazzino, char* string);
//void init_hash();
//void print_table();
//void aggiungi_ricetta(Ricetta *r);
//Ricetta* cerca_ricetta(char* nome);
//void elimina_ricetta(char* nome,coda_ordini *ordini_in_sospeso);
//Ricetta* crea_ricetta(char* s);
//Ingrediente* crea_ingrediente();
//Ingrediente* inserisci_ingrediente(Ingrediente* head, char* nome, int);
//HeapNode extract_min(IngredienteMinHeap* min_heap);
//void remove_expired_from_heap(IngredienteMinHeap* min_heap, int current_time);
//coda_ordini* crea_coda_comp();
//coda_ordini* inserisci_ordine_completo(coda_ordini* head, char* nome, int quantita, int tempo, int peso_totale);
//coda_ordini* inserisci_ordine_in_sospeso(coda_ordini* head, char* string);
//// coda_ordini* rimuovi_primo_ordine_completo(coda_ordini* head);
//void prepara_ordine(MagazzinoHashTable* magazzino, int curr_time, coda_ordini* ordini_completati, coda_ordini* ordini_in_sospeso);
//void spedisci_ordini(coda_ordini* ordini_completi, int peso_carretto);
//void ordina_per_peso(coda_ordini* head);
//void stampa_ordini(coda_ordini* head);

// Nel main leggiamo la prima stringa e poi possiamo utilizzare una funzione che anazlizza la stringa letta e a seconda che sia:
// 1)una ricetta da aggiungere/rimuovere/visualizza ingredienti;
// 2)il tempo del camioncino, ovvero ogni quanti t passa;
// 3)un elemento da aggiungere;
// chiama le rispettive funzioni, possiamo fare un semplice IF tanto si dovrebbe capire velocemente, se no un switch case.
// 1)Per le ricette optiamo per un hashmap che le contiene tutte, quindi ci basta trovare un modo efficiente
//    per creare la tabella di hash e anche una soluzione per eliminare senza problemi.
//    Possimo salvarci subito il peso della ricetta, ovvero la somma di tutti gli ingredienti, in modo tale che, quando la andiamo
//    a caricare sul camioncino, sappiamo subito il suo peso e non dobbiamo calcolaro.(peso dato dalla somma degli ingredienti)
// 2)Potremmo non creare una funzione per questo ma salvare semplicemente in una variabile il numero di T, che corrisponde a
//    ogni quanto passa il camioncino, poi ad ogni ciclo verificare se siamo nel caso %T==0 ovvero il tempo di passaggio e
//    chiamare una funzione per svolgere correttamente il caricamento.
// 3)Per la dispensa usiamo una hashmap, dobbiamo creare la key con due valori <prodotto,quantità>. Internamente creeremo un
//    albero chiamato min-heap che ordina in base alle T ovvero avremo sulla radice il T minore e via via per ogni figlio
//    da sinistra a destra in ordine crescente, complessità temporale per trovare l'elemento minimo O(1), pop e insert invece
//    è O(log(n)).
// Da capire come avvengono gli ipotetici cicli di clock, perchè secondo me ogni volta che il main legge scorre un T, ma esso
// dovrebbe essere specificato nella stringa letta.Quando le ricette sono completate non ci serve più tener traccia della scadenza
// dei prodotti utilizzati ma ci serve sapere a che T sono stati completati. Quando non riusciamo a completare un ordine si crea una
// coda, dove inseriamo le ricette in attesa, ogni volta devo verificare che gli elementi siano tornati disponibili per ogni elemento
// in coda oppure metto una flag che mi permette di capire quando determinate quantità di prodotto sono arrivate e quindi
// la ricetta è cucinabile.
// Una volta completate le ricette vengono inserite in una coda in ordine di T ovvero da quella più vecchia.
// Il carretto avrà a disposizione tot grammi caricabili, specificati nel primo messaggio con i T (funzione 2).
// Esso prende in ordine le consegne fino a che non sfora i grammi (qui non so se dobbiamo verificare se si riesce a caricare qualcosa
// di più recente ma leggero oppure lasciar stare),e successivamente riordinare la lista delle consegne in ordine di peso.
// per le stampe devo ancora vedere bene ma credo basti scrivere tutti gli ordini che riesce a stampare, le ricette quando vengono aggiunte
// e rimosse e poi non saprei.
// */

//// Funzione per estrarre il nodo con la scadenza minima dal Min-Heap
//HeapNode extract_min(IngredienteMinHeap* min_heap) {
//    if (min_heap->size <= 0) {
//        HeapNode empty_node = {0, 0};  // Nodo vuoto da restituire in caso di heap vuoto
//        return empty_node;
//    }
//    if (min_heap->size == 1) {
//        min_heap->size--;
//        return min_heap->nodes[0];
//    }
//    // Prendi la radice (minimo)
//    HeapNode root = min_heap->nodes[0];
//    // Sostituisci la radice con l'ultimo nodo e riduci la dimensione dell'heap
//    min_heap->nodes[0] = min_heap->nodes[min_heap->size - 1];
//    min_heap->size--;
//    // Riordina l'heap
//    min_heapify(min_heap, 0);
//
//    return root;
//}
////guardo solo il primo, T < curr rimuovo e vado avanti se no esco dal while, complessita o(log(n)) per rimuovere e nel caso peggiore viene fatto n volte
//void remove_expired_from_heap(IngredienteMinHeap* min_heap, int current_time) {
//    while (min_heap->size > 0 && min_heap->nodes[0].expiry < current_time) {
//        extract_min(min_heap);  // Rimuovi l'elemento scaduto
//    }
//}
//
//
//void prepara_ordine(MagazzinoHashTable* magazzino, int curr_time, coda_ordini* ordini_completati, coda_ordini* ordini_sospesi) {
//    //caso in cui chiamo prepara ordini dla rifornimento
//    if(ordini_sospesi==NULL){
//        return;
//    }
//    // Inizializza i puntatori per scorrere la coda degli ordini sospesi
//    coda_ordini* curr = ordini_sospesi;
//    coda_ordini* prec = NULL;
//
//    while (curr != NULL) {
//        char *nome_ricetta = curr->nome_ricetta;
//        int quantita = curr->quantita;
//        int tempo_richiesta = curr->tempo_richiesta;
//        Ricetta* ricetta = cerca_ricetta(curr->nome_ricetta);
//        if (!ricetta) {
//            // Ricetta non trovata, continua con il prossimo ordine
//            prec = curr;
//            curr = curr->next;
//            continue;
//        }
//
//        bool ingredienti_disponibili = true;  // Flag per verificare la disponibilità degli ingredienti
//        Ingrediente* ingrediente_corrente = ricetta->ingredienti;
//
//        // Verifica la disponibilità degli ingredienti
//        while (ingrediente_corrente != NULL) {
//            int quantita_richiesta = ordini_sospesi->quantita * ingrediente_corrente->peso;
//            IngredienteHashNode* nodo_ingrediente = magazzino->cells[hash(ingrediente_corrente->nome)];
//
//            if (!nodo_ingrediente) {
//                ingredienti_disponibili = false;  // L'ingrediente non è presente
//                break;
//            }
//
//            // Rimuove ingredienti scaduti e aggiorna il peso totale
//            remove_expired_from_heap(&nodo_ingrediente->min_heap, curr_time);
//
//            // Ricontrolla la disponibilità degli ingredienti
//            if (nodo_ingrediente->total_weight < quantita_richiesta) {
//                ingredienti_disponibili = false;  // Non c'è abbastanza quantità
//                break;
//            }
//
//            ingrediente_corrente = ingrediente_corrente->next;
//        }
//
//        if (ingredienti_disponibili) {
//            // Preparazione dell'ordine
//            int peso_totale = 0;
//            ingrediente_corrente = ricetta->ingredienti; // Ri-inizializza per il secondo ciclo
//            while (ingrediente_corrente != NULL) {
//                int quantita_richiesta = quantita * ingrediente_corrente->peso;
//                peso_totale += quantita_richiesta;
//                IngredienteHashNode* nodo_ingrediente = magazzino->cells[hash(ingrediente_corrente->nome)];
//
//                while (quantita_richiesta > 0) {
//                    HeapNode min_node = extract_min(&nodo_ingrediente->min_heap);
//                    if (min_node.weight <= quantita_richiesta) {
//                        quantita_richiesta -= min_node.weight;
//                        nodo_ingrediente->total_weight -= min_node.weight;
//                    } else {
//                        min_node.weight -= quantita_richiesta;
//                        nodo_ingrediente->total_weight -= quantita_richiesta;
//                        quantita_richiesta = 0;
//                        insert_min_heap(&nodo_ingrediente->min_heap, min_node);  // Reinserisce il nodo aggiornato
//                    }
//                }
//
//                ingrediente_corrente = ingrediente_corrente->next;
//            }
//
//            // Aggiunge l'ordine completato alla coda degli ordini completati
//            ordini_completati = inserisci_ordine_completo(ordini_completati, nome_ricetta, quantita, tempo_richiesta, peso_totale);
//
//            // Rimuove l'ordine completato dalla coda degli ordini sospesi
//            if (prec) {
//                prec->next = curr->next;
//            } else {
//                ordini_sospesi = curr->next;
//            }
//
//            coda_ordini* temp = curr;
//            curr = curr->next;
//            free(temp->nome_ricetta);
//            free(temp);
//        } else {
//            // Avanza al prossimo ordine
//            prec = curr;
//            curr = curr->next;
//        }
//    }
//}
//
//
//
////
////// funzione per rimuovere un ingrediente dalla tabella hash e dal min-heap
////void rimuovi_ingredient(MagazzinoHashTable* magazzino, char* nome_ingrediente, int current_time) {
////    int index = hash(nome_ingrediente);
////    IngredienteHashNode* ingrediente_node = magazzino->cells[index];
////    IngredienteHashNode* prev_node = NULL;
////    // trova il nodo con il nome dell'ingrediente
////    while (ingrediente_node != NULL && strcmp(ingrediente_node->key, nome_ingrediente) != 0) {
////        prev_node = ingrediente_node;
////        ingrediente_node = ingrediente_node->next;
////    }
////    // Rimuovi elementi scaduti dal min-heap
////    remove_expired_from_heap(&ingrediente_node->min_heap, current_time);
////    // Calcola il peso totale rimanente
////    int remaining_weight = 0;
////    for (int i = 0; i < ingrediente_node->min_heap.size; i++) {
////        remaining_weight += ingrediente_node->min_heap.nodes[i].weight;
////    }
////    // Aggiorna il peso totale
////    ingrediente_node->total_weight = remaining_weight;
////    // Se il peso totale è zero, elimina il nodo dalla tabella hash
////    if (ingrediente_node->total_weight == 0) {
////        if (prev_node == NULL) {
////            // Nodo da rimuovere è il primo nella lista
////            magazzino->cells[index] = ingrediente_node->next;
////        } else {
////            // Nodo da rimuovere non è il primo nella lista
////            prev_node->next = ingrediente_node->next;
////        }
////        // Libera la memoria
////        free(ingrediente_node->key);
////        free(ingrediente_node->min_heap.nodes);
////        free(ingrediente_node);
////    }
////}
//
////creo la tabella di hash

////stampo la hash

//coda_ordini* inserisci_ordine_completo(coda_ordini* head, char* nome, int quantita, int tempo, int peso){
//    coda_ordini * temp;
//    temp=crea_coda_comp();
//    temp->nome_ricetta = nome;
//    temp->quantita = quantita;
//    temp->tempo_richiesta = tempo;
//    temp->peso_totale = peso;
//    if(head == NULL){
//        head = temp;
//    }else{
//        coda_ordini* var;
//        var = head;
//        while( var->next != NULL){
//            var = var->next;
//        }
//        var->next= temp;
//    }
//    return head;
//}

////rimuovo il primo elemento, facendo si che venga deallocata la memoria
//// coda_ordini* rimuovi_primo_ordine_completo(coda_ordini* head){
////     coda_ordini* temp;
////     temp = head;
////     head = head->next;
////     coda_ordini* rimosso = crea_coda_comp();
////     rimosso->nome_ricetta = temp->nome_ricetta;
////     rimosso->tempo_richiesta = temp->tempo_richiesta;
////     free(temp->nome_ricetta);
////     free(temp);
////     return rimosso;
//// }
//void spedisci_ordini(coda_ordini* ordini_completi, int peso) {
//    coda_ordini* ordini_da_ordinare = NULL;
//    coda_ordini* current = ordini_completi;
//    coda_ordini* prev = NULL;
//    if(ordini_completi==NULL){
//        printf("camioncino vuoto\n");
//        return;
//    }
//    while (current != NULL) {
//        if (current->peso_totale <= peso) {
//            peso -= current->peso_totale;
//            coda_ordini* temp = current;
//            // rimuovo l'ordine dalla lista originale
//            if (prev == NULL) {
//                ordini_completi = current->next;  // Primo elemento
//            } else {
//                prev->next = current->next;  // Un elemento intermedio o finale
//            }
//            current = current->next;  // passo al prossimo elemento
//            // aggiungo l'ordine rimosso alla nuova coda
//            temp->next = ordini_da_ordinare;
//            ordini_da_ordinare = temp;
//        } else {
//            prev = current;
//            current = current->next;
//        }
//    }
//    // ordino la nuova coda per peso
//    ordina_per_peso(ordini_da_ordinare);
//    // stampo gli ordini ordinati
//    stampa_ordini(ordini_da_ordinare);
//    // Libera la memoria degli ordini spediti
//    while (ordini_da_ordinare != NULL) {
//        coda_ordini* temp = ordini_da_ordinare;
//        ordini_da_ordinare = ordini_da_ordinare->next;
//        free(temp);
//    }
//}
//
//void ordina_per_peso(coda_ordini* head) {
//    if (head == NULL || head->next == NULL) {
//        return; // La lista è vuota o ha un solo elemento
//    }
//
//    coda_ordini* sorted = NULL; // Lista ordinata
//    coda_ordini* current = head;
//
//    while (current != NULL) {
//        coda_ordini* next = current->next;
//        // Inserisci l'elemento corrente nella lista ordinata
//        if (sorted == NULL || sorted->peso_totale <= current->peso_totale) {
//            current->next = sorted;
//            sorted = current;
//        } else {
//            coda_ordini* temp = sorted;
//            while (temp->next != NULL && temp->next->peso_totale > current->peso_totale) {
//                temp = temp->next;
//            }
//            current->next = temp->next;
//            temp->next = current;
//        }
//        current = next;
//    }
//    head = sorted;
//}
//void stampa_ordini(coda_ordini* head) {
//    while (head != NULL) {
//        printf("%d %s %d\n", head->tempo_richiesta, head->nome_ricetta, head->quantita);
//        head = head->next;
//    }
//}
