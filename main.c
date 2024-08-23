#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_NAME 256
#define MIN_HEAP_CAPACITY 15
#define TABLE_SIZE 101
#define DELETED_NODE (ricetta*)(0xFFFFFFFFFFFFFFFUL)
char buff[100000];
unsigned long hash_string(unsigned char *str) {
    unsigned long hash = 3796;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }

    return hash % TABLE_SIZE;  // TABLE_SIZE dovrebbe essere la dimensione del tuo array di hash
}



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
        printf("coda ordini in sospeso:\n");
        stampa_coda_ordini(ordini_in_sospeso);
        printf("coda ordini completi:\n");
        stampa_coda_ordini(ordini_completi);
        print_magazzino(&magazzino);
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
//            printf("Cella %d: vuota\n", i);
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
    char* ingrediente = strdup(strtok(string, " "));
    while (ingrediente != NULL) {
        int peso_ingrediente = atoi(strtok(NULL, " "));
        int scadenza_ingrediente = atoi(strtok(NULL, " "));
        int index = hash_string(ingrediente);
        ingredienteHashNode* ingrediente_node;
        int trovato = 0;
        for (int i = 0; i < TABLE_SIZE; i++) {
            int try = (i*i + index) % TABLE_SIZE;
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
            ingredienteHashNode* nodo_ingrediente = magazzino->cells[hash_string(ingrediente_corrente->nome)];

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
                ingredienteHashNode* nodo_ingrediente = magazzino->cells[hash_string(ingrediente_corrente->nome)];

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
