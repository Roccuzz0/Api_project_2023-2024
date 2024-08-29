#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MIN_HEAP_CAPACITY 100
#define TABLE_SIZE 25323
#define DELETED_NODE (ricetta*)(0xFFFFFFFFFFFFFFUL)
char buff[25000];

typedef struct heapNode {
    int expiry;
    int weight;
} heapNode;
typedef struct ingredienteMinHeap {
    heapNode *nodes;
    int size;
    int capacity;
} ingredienteMinHeap;
typedef struct ingredienteHashNode {
    char *nome;
    int total_weight;
    ingredienteMinHeap min_heap;
} ingredienteHashNode;
typedef struct coda_ingredienti {
    ingredienteHashNode* ingrediente;
    int peso;
    struct coda_ingredienti* next;
}coda_ingredienti;
typedef struct ricetta {
    char *nome;
    int in_sospeso;
    coda_ingredienti *ingredienti;
} ricetta;
typedef struct coda_ordini_completi{
    char* nome_ricetta;
    int quantita;
    int tempo_richiesta;
    int peso_totale;
    struct coda_ordini_completi* next;
}coda_ordini_completi;
typedef struct coda_ordini_in_sospeso{
    ricetta* ricetta;
    int quantita;
    int tempo_richiesta;
    struct coda_ordini_in_sospeso* next;
}coda_ordini_in_sospeso;
typedef struct lista_ricetta_infattibile {
    struct ricetta* ricetta;
    int quantita;
    struct lista_ricetta_infattibile* next;
} lista_ricetta_infattibile;
ricetta* ricette_hash_table[TABLE_SIZE] = {NULL};
ingredienteHashNode* magazzino[TABLE_SIZE] = {NULL};
lista_ricetta_infattibile* ric_inf = {NULL};
coda_ordini_completi* head_ordini_completi = NULL;
coda_ordini_completi* tail_ordini_completi = NULL;
coda_ordini_in_sospeso* head_ordini_in_sospeso = NULL;
coda_ordini_in_sospeso* tail_ordini_in_sospeso = NULL;


void singolo_ordine(int t,char* funz);
ricetta* cerca_ricetta(char *nome_ricetta);
void aggiungi_ricetta(char* funz);
ricetta *crea_ricetta(char* nome_ricetta);
coda_ingredienti* inserisci_ingrediente(coda_ingredienti * head, char* nome, int peso);
coda_ingredienti * crea_ingrediente();
void elimina_ricetta(char *nome);
void rifornimento(char* string,int tempo);
void insert_min_heap(ingredienteMinHeap* min_heap, heapNode node);
void trim_trailing_whitespace(char* str);
void inserisci_ordine_in_sospeso(coda_ordini_in_sospeso *temp);
heapNode extract_min(ingredienteMinHeap* min_heap);
void prepara_ordine(int curr_time);
void remove_expired_from_heap(ingredienteHashNode* nodo, int current_time);
void inserisci_ordine_completo( char* nome, int quantita, int tempo, int peso);
void spedisci_ordini( int peso);
coda_ordini_completi* ordina_per_peso(coda_ordini_completi * head);
void stampa_ordini(coda_ordini_completi * head);


int main(){
    int t = 0;
    int tempo_carretto,peso_carretto;
    if (scanf("%d %d\n", &tempo_carretto, &peso_carretto) != 2) {
        return 1;
    }
    do{
        if(t % tempo_carretto==0 && t != 0){
            spedisci_ordini(peso_carretto);
        }
        char *result = fgets(buff, sizeof(buff), stdin);
        (void)result;
        char* token = strtok(buff," ");
        char* funz = strtok(NULL, "");
        if(funz == NULL){
            break;
        }
        if(strcmp(token,"aggiungi_ricetta")==0){
            aggiungi_ricetta(funz);
        }
        else if(strcmp(token,"rimuovi_ricetta")==0){
            elimina_ricetta(funz);
        }
        else if(strcmp(token,"rifornimento")==0){
            rifornimento(funz,t);
            prepara_ordine(t);
        }
        else if(strcmp(token,"ordine")==0){
            singolo_ordine(t,funz);
        }
        t++;
    }while(1);
    return 0;
}
unsigned long hash_string(char *str) {
    unsigned long hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % TABLE_SIZE;
}
void singolo_ordine(int tempo, char* funz) {
    char* nome = strtok(funz, " ");
    int quantita = atoi(strtok(NULL, " "));
    int index = hash_string(nome);
    int ricetta_trovata = 0;
    struct ricetta* ric = NULL;
    for (int i = 0; i < TABLE_SIZE; i++) {
        int try = (i + index) % TABLE_SIZE;
        if (ricette_hash_table[try] != NULL && ricette_hash_table[try] != DELETED_NODE) {
            if (strcmp(ricette_hash_table[try]->nome, nome) == 0) {
                ricetta_trovata = 1;
                ric = ricette_hash_table[try];
                ricette_hash_table[try]->in_sospeso ++;
                break;
            }
        }
    }
    if (ricetta_trovata == 0) {
        printf("rifiutato\n");
        return;
    }
    printf("accettato\n");
    lista_ricetta_infattibile* ric_inf_corrente = ric_inf;
    while(ric_inf_corrente!=NULL){
        if(ric_inf_corrente->ricetta == ric && ric_inf_corrente->quantita <= quantita){
            coda_ordini_in_sospeso * temp;
            temp = (coda_ordini_in_sospeso*)malloc(sizeof(coda_ordini_in_sospeso));
            if (temp == NULL) {
                return;
            }
            temp->ricetta = ric;
            temp->quantita = quantita;
            temp->tempo_richiesta = tempo;
            temp->next = NULL;
            inserisci_ordine_in_sospeso(temp);
            return;
        }
        ric_inf_corrente=ric_inf_corrente->next;
    }
    int ingredienti_disponibili = 1;
    coda_ingredienti* ingrediente_corrente = ric->ingredienti;
    while (ingrediente_corrente != NULL) {
        int quantita_richiesta = quantita * ingrediente_corrente->peso;
        ingredienteHashNode* nodo_ingrediente = magazzino[hash_string(ingrediente_corrente->ingrediente->nome)];
        if (!nodo_ingrediente) {
            ingredienti_disponibili = 0;
            break;
        }
        remove_expired_from_heap(nodo_ingrediente, tempo);
        if (nodo_ingrediente->total_weight < quantita_richiesta) {
            ingredienti_disponibili = 0;
            break;
        }
        ingrediente_corrente = ingrediente_corrente->next;
    }
    if (ingredienti_disponibili) {
        ingrediente_corrente = ric->ingredienti;
        while (ingrediente_corrente != NULL) {
            int quantita_richiesta = quantita * ingrediente_corrente->peso;
            ingredienteHashNode* nodo_ingrediente = magazzino[hash_string(ingrediente_corrente->ingrediente->nome)];
            while (quantita_richiesta > 0) {
                heapNode min_node = extract_min(&nodo_ingrediente->min_heap);
                if (min_node.weight <= quantita_richiesta) {
                    quantita_richiesta -= min_node.weight;
                    nodo_ingrediente->total_weight -= min_node.weight;
                } else {
                    min_node.weight -= quantita_richiesta;
                    nodo_ingrediente->total_weight -= quantita_richiesta;
                    quantita_richiesta = 0;
                    if (min_node.weight > 0) {
                        insert_min_heap(&nodo_ingrediente->min_heap, min_node);
                    }
                }
            }
            ingrediente_corrente = ingrediente_corrente->next;
        }
        int peso_totale = 0;
        ingrediente_corrente = ric->ingredienti;
        while (ingrediente_corrente != NULL) {
            peso_totale += quantita * ingrediente_corrente->peso;
            ingrediente_corrente = ingrediente_corrente->next;
        }
        inserisci_ordine_completo(nome, quantita, tempo, peso_totale);
        return;
    } else {
        coda_ordini_in_sospeso* temp = (coda_ordini_in_sospeso*)malloc(sizeof(coda_ordini_in_sospeso));
        if (temp == NULL) {
            return;
        }
        temp->ricetta = (ricetta*)malloc(sizeof(ricetta));
        if (temp->ricetta == NULL) {
            free(temp);
            return;
        }
        temp->ricetta->ingredienti = (coda_ingredienti*)malloc(sizeof(coda_ingredienti));
        if (temp->ricetta->ingredienti == NULL) {
            free(temp->ricetta);
            free(temp);
            return;
        }
        temp->ricetta->nome=ric->nome;
        temp->ricetta->ingredienti=ric->ingredienti;
        temp->quantita = quantita;
        temp->tempo_richiesta = tempo;
        temp->next = NULL;
        inserisci_ordine_in_sospeso(temp);
    }
}

void aggiungi_ricetta(char* funz) {
    char* nome_ricetta = strtok(funz," ");
    if(cerca_ricetta(nome_ricetta)!=NULL){
        printf("ignorato\n");
        return;
    }
    ricetta* r = crea_ricetta(nome_ricetta);
    int index = hash_string(r->nome);
    for (int i = 0; i < TABLE_SIZE; i++) {
        int try = (index + i) % TABLE_SIZE;
        if (ricette_hash_table[try] == NULL || ricette_hash_table[try] == DELETED_NODE) {
            ricette_hash_table[try] = r;
            ricette_hash_table[try]->in_sospeso = 0;
            printf("aggiunta\n");
            break;
        }
    }
}
ricetta *cerca_ricetta(char *nome_ricetta){
    int index = hash_string(nome_ricetta);
    for (int i = 0; i < TABLE_SIZE; i++) {
        int try = (i + index) % TABLE_SIZE;
        if(ricette_hash_table[try]==NULL){
            return NULL;
        }
        if(ricette_hash_table[try]==DELETED_NODE) continue;
        if (strcmp(ricette_hash_table[try]->nome, nome_ricetta)==0){
            return ricette_hash_table[try];
        }
    }
    return NULL;
}

ricetta *crea_ricetta(char* nome_ricetta){
    ricetta *nuova_ricetta = (ricetta*)malloc(sizeof(ricetta));
    if(nuova_ricetta == NULL){
        return NULL;
    }
    nuova_ricetta->nome = strdup(nome_ricetta);
    if(nuova_ricetta->nome== NULL){
        free(nuova_ricetta);
        return NULL;
    }
    coda_ingredienti *head = NULL;
    char* token = strtok(NULL, " ");
    if (token == NULL) {
        free(nuova_ricetta->nome);
        free(nuova_ricetta);
        return NULL;
    }
    char* nome_ingrediente = token;
    token = strtok(NULL, " ");
    if (token == NULL) {
        free(nuova_ricetta->nome);
        free(nuova_ricetta);
        return NULL;
    }
    int peso = atoi(token);
    head = inserisci_ingrediente(head, nome_ingrediente, peso);
    token = strtok(NULL, " ");
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
    temp->ingrediente->nome = strdup(nome);
    if(temp->ingrediente->nome==NULL){
        free(nome);
        free(temp);
        return head;
    }
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
    if(temp==NULL){
        return NULL;
    }
    temp->ingrediente = (ingredienteHashNode*) malloc(sizeof (ingredienteHashNode));
    temp->next = NULL;
    return temp;
}

void elimina_ricetta(char *nome_ricetta) {
    char* nome = strtok(nome_ricetta," ");
    trim_trailing_whitespace(nome_ricetta);
    int index = hash_string(nome);
    for (int i = 0; i < TABLE_SIZE; i++) {
        int try = (i + index) % TABLE_SIZE;
        if (ricette_hash_table[try] == NULL) {
            printf("non presente\n");
            return;
        }
        if (ricette_hash_table[try] == DELETED_NODE) continue;
        if (strcmp(ricette_hash_table[try]->nome, nome) == 0) {
            if(ricette_hash_table[try]->in_sospeso == 0){
                ricette_hash_table[try] = DELETED_NODE;
                printf("rimossa\n");
                return;
            }else{
                printf("ordini in sospeso\n");
                return;
            }
        }
    }
}

void rifornimento(char* string, int tempo) {
    char* ingrediente = strtok(string, " ");
    while (ingrediente != NULL) {
        int peso_ingrediente = atoi(strtok(NULL, " "));
        int scadenza_ingrediente = atoi(strtok(NULL, " "));
        if (scadenza_ingrediente <= tempo || peso_ingrediente <= 0) {
            ingrediente = strtok(NULL, " ");
            continue;
        }
        int index = hash_string(ingrediente);
        ingredienteHashNode* ingrediente_node = NULL;
        int trovato = 0;
        for (int i = 0; i < TABLE_SIZE; i++) {
            int try = (index + i) % TABLE_SIZE;
            if (magazzino[try] == NULL) {
                ingrediente_node = (ingredienteHashNode*)malloc(sizeof(ingredienteHashNode));
                if (ingrediente_node == NULL) {
                    return;
                }
                ingrediente_node->nome = strdup(ingrediente);
                if (ingrediente_node->nome == NULL) {
                    free(ingrediente_node);
                    return;
                }
                ingrediente_node->total_weight = 0;
                ingrediente_node->min_heap.size = 0;
                ingrediente_node->min_heap.capacity = MIN_HEAP_CAPACITY;
                ingrediente_node->min_heap.nodes = (heapNode*)malloc(ingrediente_node->min_heap.capacity * sizeof(heapNode));
                if (ingrediente_node->min_heap.nodes == NULL) {
                    free(ingrediente_node->nome);
                    free(ingrediente_node);
                    return;
                }
                magazzino[try] = ingrediente_node;
                trovato = 1;
                break;
            } else if (strcmp(magazzino[try]->nome, ingrediente) == 0) {
                ingrediente_node = magazzino[try];
                trovato = 1;
                break;
            }
        }
        if (!trovato) {
            printf("Spazio nel magazzino esaurito.\n");
            return;
        }
        ingrediente_node->total_weight += peso_ingrediente;
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
    if (left < min_heap->size && min_heap->nodes[left].expiry < min_heap->nodes[smallest].expiry) {
        smallest = left;
    }
    if (right < min_heap->size && min_heap->nodes[right].expiry < min_heap->nodes[smallest].expiry) {
        smallest = right;
    }
    if (smallest != index) {
        swap_node(&min_heap->nodes[smallest], &min_heap->nodes[index]);
        min_heapify(min_heap, smallest);
    }
}

void insert_min_heap(ingredienteMinHeap* min_heap, heapNode node) {
    if (min_heap->size == min_heap->capacity) {
        min_heap->capacity *= 2;
        heapNode *new_nodes = (heapNode *)realloc(min_heap->nodes, min_heap->capacity * sizeof(heapNode));
        if (new_nodes == NULL) {
            return;
        }
        min_heap->nodes = new_nodes;
    }
    int index = min_heap->size++;
    min_heap->nodes[index] = node;
    while (index != 0 && min_heap->nodes[(index - 1) / 2].expiry > min_heap->nodes[index].expiry) {
        swap_node(&min_heap->nodes[index], &min_heap->nodes[(index - 1) / 2]);
        index = (index - 1) / 2;
    }
}

void inserisci_ordine_in_sospeso(coda_ordini_in_sospeso * temp) {
    if (tail_ordini_in_sospeso == NULL) {
        head_ordini_in_sospeso = temp;
        tail_ordini_in_sospeso = temp;
    } else {
        tail_ordini_in_sospeso->next = temp;
        tail_ordini_in_sospeso = temp;
    }
}

void prepara_ordine(int curr_time) {
    if (head_ordini_in_sospeso == NULL) {
        return ;
    }
    lista_ricetta_infattibile* infattibili = (lista_ricetta_infattibile*) malloc(sizeof (lista_ricetta_infattibile));
    if (infattibili == NULL) {
        return;
    }
    ric_inf=NULL;

    coda_ordini_in_sospeso *curr = head_ordini_in_sospeso;
    coda_ordini_in_sospeso *prec = NULL;
    bool infat = false;
    while (curr != NULL) {
            while (ric_inf != NULL) {
                if (ric_inf->ricetta == curr->ricetta && ric_inf->quantita <= curr->quantita) {
                    infat = true;
                    break;
                }
                ric_inf = ric_inf->next;
            }
        if (infat) {
            curr = curr->next;
            continue;
        }
        int quantita = curr->quantita;
        int tempo_richiesta = curr->tempo_richiesta;
        int ingredienti_disponibili = 1;
        coda_ingredienti *ingrediente_corrente = curr->ricetta->ingredienti;
        while (ingrediente_corrente != NULL) {
            int peso = ingrediente_corrente->peso;
            int quantita_richiesta = quantita * peso;
            ingredienteHashNode *nodo_ingrediente = magazzino[hash_string(ingrediente_corrente->ingrediente->nome)];
            if (nodo_ingrediente == NULL) {
                ingredienti_disponibili = 0;
                break;
            }
            remove_expired_from_heap(nodo_ingrediente, curr_time);
            if (nodo_ingrediente->total_weight < quantita_richiesta) {
                ingredienti_disponibili = 0;
                break;
            }
            ingrediente_corrente = ingrediente_corrente->next;
        }
        if (!ingredienti_disponibili) {
            if(ric_inf==NULL){
                infattibili->ricetta = curr->ricetta;
                infattibili->quantita = curr->quantita;
                infattibili->next = ric_inf;
                ric_inf = infattibili;
            }else{
                while(ric_inf!=NULL){
                    if(ric_inf->ricetta==curr->ricetta){
                        ric_inf->quantita=curr->quantita;
                        break;
                    }
                    ric_inf=ric_inf->next;
                }
            }
        }
        if (ingredienti_disponibili) {
            int peso_totale = 0;
            ingrediente_corrente = curr->ricetta->ingredienti;
            while (ingrediente_corrente != NULL) {
                int quantita_richiesta = quantita * ingrediente_corrente->peso;
                ingredienteHashNode *nodo_ingrediente = magazzino[hash_string(
                        ingrediente_corrente->ingrediente->nome)];
                while (quantita_richiesta > 0) {
                    heapNode min_node = extract_min(&nodo_ingrediente->min_heap);
                    if (min_node.weight <= quantita_richiesta) {
                        quantita_richiesta -= min_node.weight;
                        nodo_ingrediente->total_weight -= min_node.weight;
                    } else {
                        min_node.weight -= quantita_richiesta;
                        nodo_ingrediente->total_weight -= quantita_richiesta;
                        quantita_richiesta = 0;
                        if (min_node.weight > 0) {
                            insert_min_heap(&nodo_ingrediente->min_heap, min_node);
                        }
                    }
                }
                peso_totale += quantita * ingrediente_corrente->peso;
                ingrediente_corrente = ingrediente_corrente->next;
            }
            inserisci_ordine_completo(curr->ricetta->nome, quantita, tempo_richiesta, peso_totale);
            if (prec) {
                prec->next = curr->next;
            } else {
                head_ordini_in_sospeso = curr->next;
            }
            if (curr->next == NULL) {
                tail_ordini_in_sospeso = prec;
            }
            coda_ordini_in_sospeso *temp = curr;
            curr = curr->next;
            free(temp->ricetta);
            free(temp);
        } else {
            prec = curr;
            curr = curr->next;
        }
    }
}



heapNode extract_min(ingredienteMinHeap* min_heap) {
    if (min_heap->size <= 0) {
        heapNode empty_node = {0, 0};
        return empty_node;
    }
    if (min_heap->size == 1) {
        min_heap->size--;
        return min_heap->nodes[0];
    }
    heapNode root = min_heap->nodes[0];
    min_heap->nodes[0] = min_heap->nodes[min_heap->size - 1];
    min_heap->size--;
    min_heapify(min_heap, 0);
    return root;
}

void remove_expired_from_heap(ingredienteHashNode* nodo, int current_time) {
    while (nodo->min_heap.size > 0 &&
           (nodo->min_heap.nodes[0].expiry <= current_time || nodo->min_heap.nodes[0].expiry == 0)) {
        heapNode resto = extract_min(&nodo->min_heap);
        nodo->total_weight -= resto.weight;
    }
}

void inserisci_ordine_completo(char* nome, int quantita, int tempo, int peso) {
    coda_ordini_completi * temp = (coda_ordini_completi*) malloc(sizeof (coda_ordini_completi));
    if(temp==NULL){
        return;
    }
    temp->nome_ricetta = strdup(nome);
    if(temp->nome_ricetta == NULL) {
        free(temp);
        return;
    }
    temp->quantita = quantita;
    temp->tempo_richiesta = tempo;
    temp->peso_totale = peso;
    temp->next = NULL;
    if (head_ordini_completi == NULL || head_ordini_completi->tempo_richiesta > tempo) {
        temp->next = head_ordini_completi;
        head_ordini_completi = temp;
        if (tail_ordini_completi == NULL) {
            tail_ordini_completi = temp;
        }
    } else {
        coda_ordini_completi * current = head_ordini_completi;
        while (current->next != NULL && current->next->tempo_richiesta <= tempo) {
            current = current->next;
        }
        temp->next = current->next;
        current->next = temp;
    }
    if (temp->next == NULL) {
        tail_ordini_completi = temp;
    }
}

void spedisci_ordini(int peso_carretto) {
    if (head_ordini_completi == NULL) {
        printf("camioncino vuoto\n");
        return;
    }
    coda_ordini_completi* ordini_da_ordinare = NULL;
    coda_ordini_completi* temp_ordinare = NULL;
    coda_ordini_completi* current = head_ordini_completi;
    while (current != NULL) {
        if (current->peso_totale <= peso_carretto) {
            peso_carretto -= current->peso_totale;
            int index = hash_string(current->nome_ricetta);
            for (int i = 0; i < TABLE_SIZE; i++) {
                int try = (i + index) % TABLE_SIZE;
                if (ricette_hash_table[try] != NULL && ricette_hash_table[try] != DELETED_NODE) {
                    if (strcmp(ricette_hash_table[try]->nome, current->nome_ricetta) == 0) {
                        ricette_hash_table[try]->in_sospeso --;
                        break;
                    }
                }
            }
            coda_ordini_completi* next_node = current->next;
            current->next = temp_ordinare;
            temp_ordinare = current;
            current = next_node;
        } else {
            break;
        }
    }
    head_ordini_completi = current;
    ordini_da_ordinare = ordina_per_peso(temp_ordinare);
    stampa_ordini(ordini_da_ordinare);
    while (ordini_da_ordinare != NULL) {
        coda_ordini_completi* temp = ordini_da_ordinare;
        ordini_da_ordinare = ordini_da_ordinare->next;
        free(temp);
    }
}

coda_ordini_completi* ordina_per_peso(coda_ordini_completi* head) {
    if (head == NULL || head->next == NULL) {
        return head;
    }
    coda_ordini_completi* sorted = NULL;
    while (head != NULL) {
        coda_ordini_completi* current = head;
        head = head->next;
        if (sorted == NULL ||
            (sorted->peso_totale < current->peso_totale) ||
            (sorted->peso_totale == current->peso_totale && sorted->tempo_richiesta > current->tempo_richiesta)) {
            current->next = sorted;
            sorted = current;
        } else {
            coda_ordini_completi* temp = sorted;
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

void stampa_ordini(coda_ordini_completi* head) {
    if (head == NULL) {
        printf("Nessun ordine da stampare\n");
        return;
    }
    while (head != NULL) {
        printf("%d %s %d\n", head->tempo_richiesta, head->nome_ricetta, head->quantita);
        head = head->next;
    }
}