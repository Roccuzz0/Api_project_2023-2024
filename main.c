#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAME 20
#define TABLE_SIZE 1003
#define RICETTE_SIZE 24893
#define DELETED_NODE (ricetta*)(0xFFFFFFFFFFFFFFUL)
char buff[2983];

typedef struct nodo_coda {
    int expiry;
    int weight;
    struct nodo_coda* next;
} nodo_coda;
typedef struct ingredienteHashNode {
    char nome[NAME];
    int index_table;
    int total_weight;
    nodo_coda* head;
} ingredienteHashNode;
typedef struct coda_ingredienti {
    ingredienteHashNode* ingrediente;
    int peso;
    struct coda_ingredienti* next;
}coda_ingredienti;
typedef struct ricetta {
    char nome[NAME];
    int in_sospeso;
    coda_ingredienti *ingredienti;
} ricetta;
typedef struct coda_ordini_completi{
    char nome_ricetta[NAME];
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

ricetta* ricette_hash_table[RICETTE_SIZE] = {NULL};
ingredienteHashNode* magazzino[TABLE_SIZE] = {NULL};
coda_ordini_completi* head_ordini_completi = NULL;
coda_ordini_completi* tail_ordini_completi = NULL;
coda_ordini_in_sospeso* head_ordini_in_sospeso = NULL;
coda_ordini_in_sospeso* tail_ordini_in_sospeso = NULL;

void rimuovi_ingredienti_per_ordine(ingredienteHashNode* nodo_ingrediente, int quantita_richiesta);
void rimuovi_ingredienti_scaduti(ingredienteHashNode* nodo, int tempo_corrente);
void inserisci_in_coda_ingredienti(ingredienteHashNode* nodo, int scadenza_ingrediente, int peso_ingrediente);
unsigned long hash_string(char *str,int dimensione);
void singolo_ordine(int t,char* funz);
ricetta* cerca_ricetta(char *nome_ricetta);
void aggiungi_ricetta(char* funz);
ricetta *crea_ricetta(char nome_ricetta[NAME]);
coda_ingredienti* inserisci_ingrediente(coda_ingredienti * head, char nome[NAME], int peso);
coda_ingredienti * crea_ingrediente();
void elimina_ricetta(char nome[NAME]);
void rifornimento(char* string,int tempo);
void trim_trailing_whitespace(char* str);
void inserisci_ordine_in_sospeso(coda_ordini_in_sospeso *temp);
void prepara_ordine(int curr_time);
void inserisci_ordine_completo( char nome[NAME], int quantita, int tempo, int peso);
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

unsigned long hash_string(char *str,int dimensione) {
    unsigned long hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % dimensione;
}

void singolo_ordine(int tempo, char* funz) {
    char* nome = strtok(funz, " ");
    int quantita = atoi(strtok(NULL, " "));
    int index = hash_string(nome, RICETTE_SIZE);
    int ricetta_trovata = 0;
    struct ricetta* ric = NULL;
    for (int i = 0; i < RICETTE_SIZE; i++) {
        int try = (i + index) % RICETTE_SIZE;
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
    int ingredienti_disponibili = 1;
    coda_ingredienti* ingrediente_corrente = ric->ingredienti;
    while (ingrediente_corrente) {
        int quantita_richiesta = quantita * ingrediente_corrente->peso;
        if(ingrediente_corrente->ingrediente->index_table == 0){
            ingredienti_disponibili = 0;
            break;
        }
        ingredienteHashNode* nodo_ingrediente = magazzino[ingrediente_corrente->ingrediente->index_table];
        rimuovi_ingredienti_scaduti(nodo_ingrediente, tempo);
        if (nodo_ingrediente->total_weight < quantita_richiesta) {
            ingredienti_disponibili = 0;
            break;
        }
        ingrediente_corrente = ingrediente_corrente->next;
    }
    if (ingredienti_disponibili) {
        ingrediente_corrente = ric->ingredienti;
        while (ingrediente_corrente) {
            int quantita_richiesta = quantita * ingrediente_corrente->peso;
            ingredienteHashNode* nodo_ingrediente = magazzino[ingrediente_corrente->ingrediente->index_table];
            rimuovi_ingredienti_per_ordine(nodo_ingrediente,quantita_richiesta);
            ingrediente_corrente = ingrediente_corrente->next;
        }
        int peso_totale = 0;
        ingrediente_corrente = ric->ingredienti;
        while (ingrediente_corrente) {
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
        strcpy(temp->ricetta->nome,ric->nome);
        temp->ricetta->ingredienti=ric->ingredienti;
        temp->quantita = quantita;
        temp->tempo_richiesta = tempo;
        temp->next = NULL;
        inserisci_ordine_in_sospeso(temp);
    }
}

void rimuovi_ingredienti_per_ordine(ingredienteHashNode* nodo_ingrediente, int quantita_richiesta) {
    nodo_coda* corrente = nodo_ingrediente->head;
    while (quantita_richiesta > 0 && corrente != NULL) {
        if (corrente->weight <= quantita_richiesta) {
            quantita_richiesta -= corrente->weight;
            nodo_ingrediente->total_weight -= corrente->weight;
            nodo_ingrediente->head = corrente->next;
            nodo_coda* da_rimuovere = corrente;
            corrente = corrente->next;
            free(da_rimuovere);
        } else {
            corrente->weight -= quantita_richiesta;
            nodo_ingrediente->total_weight -= quantita_richiesta;
            quantita_richiesta = 0;
        }
    }
}

void aggiungi_ricetta(char* funz) {
    char* nome_ricetta = strtok(funz," ");
    if(cerca_ricetta(nome_ricetta)){
        printf("ignorato\n");
        return;
    }
    ricetta* r = crea_ricetta(nome_ricetta);
    int index = hash_string(r->nome,RICETTE_SIZE);
    for (int i = 0; i < RICETTE_SIZE; i++) {
        int try = (index + i) % RICETTE_SIZE;
        if (ricette_hash_table[try] == NULL || ricette_hash_table[try] == DELETED_NODE) {
            ricette_hash_table[try] = r;
            ricette_hash_table[try]->in_sospeso = 0;
            printf("aggiunta\n");
            break;
        }
    }
}

ricetta *cerca_ricetta(char *nome_ricetta){
    int index = hash_string(nome_ricetta,RICETTE_SIZE);
    for (int i = 0; i < RICETTE_SIZE; i++) {
        int try = (i + index) % RICETTE_SIZE;
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
    strcpy(nuova_ricetta->nome,nome_ricetta);
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
    while(token ){
        nome_ingrediente = token;
        token = strtok(NULL, " ");
        peso = atoi(token);
        head = inserisci_ingrediente(head, nome_ingrediente, peso);
        token = strtok(NULL, " ");
    }
    nuova_ricetta->ingredienti = head;
    return nuova_ricetta;
}

coda_ingredienti* inserisci_ingrediente(coda_ingredienti* head, char nome[NAME], int peso){
    coda_ingredienti* temp;
    int i = 0;
    temp = crea_ingrediente();
        int index = hash_string(nome, TABLE_SIZE);
        while(i<TABLE_SIZE) {
            int try = (index + i) % TABLE_SIZE;
            if (magazzino[try] == NULL) {
                ingredienteHashNode *ingrediente_node = (ingredienteHashNode*)malloc(sizeof(ingredienteHashNode));
                strcpy(ingrediente_node->nome,nome);
                ingrediente_node->total_weight = 0;
                ingrediente_node->head = (nodo_coda*)malloc(sizeof (nodo_coda));
                ingrediente_node->index_table = try;
                magazzino[try] = ingrediente_node;
                temp->ingrediente = ingrediente_node;
                break;
            }else if(strcmp(nome, magazzino[try]->nome)==0){
                temp->ingrediente = magazzino[try];
                break;
            }
            i++;
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
    if(!temp){
        return NULL;
    }
    temp->ingrediente = (ingredienteHashNode*) malloc(sizeof (ingredienteHashNode));
    if(!temp->ingrediente){
        free(temp);
        return NULL;
    }
    temp->next = NULL;
    return temp;
}

void elimina_ricetta(char *nome_ricetta) {
    char* nome = strtok(nome_ricetta," ");
    trim_trailing_whitespace(nome_ricetta);
    int index = hash_string(nome,RICETTE_SIZE);
    for (int i = 0; i < RICETTE_SIZE; i++) {
        int try = (i + index) % RICETTE_SIZE;
        if (ricette_hash_table[try] == NULL) {
            printf("non presente\n");
            return;
        }
        if (ricette_hash_table[try] == DELETED_NODE) continue;
        if (strcmp(ricette_hash_table[try]->nome, nome) == 0) {
            if(ricette_hash_table[try]->in_sospeso == 0){
                free(ricette_hash_table[try]->nome);
                free(ricette_hash_table[try]->ingredienti);
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
        int index = hash_string(ingrediente, TABLE_SIZE);
        ingredienteHashNode* ingrediente_node = NULL;
        int trovato = 0;
        for (int i = 0; i < TABLE_SIZE; i++) {
            int try = (index + i) % TABLE_SIZE;
            if (magazzino[try] == NULL) {
                ingrediente_node = (ingredienteHashNode*)malloc(sizeof(ingredienteHashNode));
                if (!ingrediente_node) {
                    return;
                }
                strcpy(ingrediente_node->nome,ingrediente);
                ingrediente_node->total_weight = 0;
                ingrediente_node->head = (nodo_coda*)malloc(sizeof (nodo_coda));
                if(!ingrediente_node->head){
                    free(ingrediente_node);
                    return;
                }
                ingrediente_node->index_table = try;
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
            return;
        }
        ingrediente_node->total_weight += peso_ingrediente;
        inserisci_in_coda_ingredienti(ingrediente_node, scadenza_ingrediente, peso_ingrediente);
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

void inserisci_in_coda_ingredienti(ingredienteHashNode* nodo, int scadenza_ingrediente, int peso_ingrediente) {
    nodo_coda* nuovoNodo = (nodo_coda*)malloc(sizeof(nodo_coda));
    if (nuovoNodo == NULL) {
        return;
    }
    nuovoNodo->expiry = scadenza_ingrediente;
    nuovoNodo->weight = peso_ingrediente;
    nuovoNodo->next = NULL;
    if (nodo->head == NULL) {
        nodo->head = nuovoNodo;
    } else {
        nodo_coda* corrente = nodo->head;
        nodo_coda* precedente = NULL;
        while (corrente != NULL && corrente->expiry < scadenza_ingrediente) {
            precedente = corrente;
            corrente = corrente->next;
        }
        if (precedente == NULL) {
            nuovoNodo->next = nodo->head;
            nodo->head = nuovoNodo;
        } else {
            precedente->next = nuovoNodo;
            nuovoNodo->next = corrente;
        }
    }
}

void rimuovi_ingredienti_scaduti(ingredienteHashNode* nodo, int tempo_corrente) {
    if (nodo == NULL || nodo->head == NULL) {
        return;
    }
    nodo_coda* corrente = nodo->head;
    nodo_coda* precedente = NULL;
    while (corrente != NULL) {
        if (corrente->expiry <= tempo_corrente) {
            if (precedente == NULL) {
                nodo->head = corrente->next;
            } else {
                precedente->next = corrente->next;
            }
            nodo->total_weight -= corrente->weight;
            nodo_coda* da_rimuovere = corrente;
            corrente = corrente->next;
            free(da_rimuovere);
        } else {
            precedente = corrente;
            corrente = corrente->next;
        }
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
    for(int i=0;i<TABLE_SIZE;i++){
        if(magazzino[i]==NULL){
            continue;
        }
        rimuovi_ingredienti_scaduti(magazzino[i],curr_time);
    }
    coda_ordini_in_sospeso *curr = head_ordini_in_sospeso;
    coda_ordini_in_sospeso *prec = NULL;
    while (curr ) {
        int quantita = curr->quantita;
        int tempo_richiesta = curr->tempo_richiesta;
        int ingredienti_disponibili = 1;
        coda_ingredienti *ingrediente_corrente = curr->ricetta->ingredienti;
        while (ingrediente_corrente != NULL) {
            int peso = ingrediente_corrente->peso;
            int quantita_richiesta = quantita * peso;
            if(ingrediente_corrente->ingrediente->index_table == 0){
                ingredienti_disponibili = 0;
                break;
            }
            ingredienteHashNode *nodo_ingrediente = magazzino[ingrediente_corrente->ingrediente->index_table];
//            if (nodo_ingrediente == NULL) {
//                ingredienti_disponibili = 0;
//                break;
//            }
            if (nodo_ingrediente->total_weight < quantita_richiesta) {
                ingredienti_disponibili = 0;
                break;
            }
            ingrediente_corrente = ingrediente_corrente->next;
        }
        if (ingredienti_disponibili) {
            int peso_totale = 0;
            ingrediente_corrente = curr->ricetta->ingredienti;
            while (ingrediente_corrente != NULL) {
                int quantita_richiesta = quantita * ingrediente_corrente->peso;
                ingredienteHashNode *nodo_ingrediente = magazzino[ingrediente_corrente->ingrediente->index_table];
                rimuovi_ingredienti_per_ordine(nodo_ingrediente,quantita_richiesta);
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

void inserisci_ordine_completo(char nome[NAME], int quantita, int tempo, int peso) {
    coda_ordini_completi * temp = (coda_ordini_completi*) malloc(sizeof (coda_ordini_completi));
    if(!temp){
        return;
    }
    strcpy(temp->nome_ricetta,nome);
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
            int index = hash_string(current->nome_ricetta,RICETTE_SIZE);
            for (int i = 0; i < RICETTE_SIZE; i++) {
                int try = (i + index) % RICETTE_SIZE;
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
    coda_ordini_completi* temp;
    while (ordini_da_ordinare != NULL) {
        temp = ordini_da_ordinare;
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
    while (head != NULL) {
        printf("%d %s %d\n", head->tempo_richiesta, head->nome_ricetta, head->quantita);
        head = head->next;
    }
}