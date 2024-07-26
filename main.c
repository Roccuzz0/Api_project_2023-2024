#include <stdio.h>
int main(){
    printf("Hello World!");
    return 0;
}

/*
 Nel main leggiamo la prima stringa e poi possiamo utilizzare una funzione che anazlizza la stringa letta e a seconda che sia:
 1)una ricetta da aggiungere/rimuovere;
 2)il tempo del camioncino, ovvero ogni quanti t passa;
 3)un elemento da aggiungere;
 chiama le rispettive funzioni, possiamo fare un semplice IF tanto si dovrebbe capire velocemente, se no un switch case.
 1)Per le ricette optiamo per un hashmap che le contiene tutte, quindi ci basta trovare un modo efficiente
    per creare la tabella di hash e anche una soluzione per eliminare senza problemi.
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