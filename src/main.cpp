#include <iostream>
#include <vector>
#include <cstdlib> //zawiera rand() do generowania liczb lososwych
#include <ctime> //zawiera time(), które jest potrzebne do inicjowania generatora liczb losowych
#include <algorithm> // zawiera std::none_of
using namespace std;

// Funkcja do czyszczenia ekranu
void clearScreen() {
    #if defined(_WIN32)
        system("cls");
    #else
        system("clear");
    #endif
}

//definicja wyjątków - w std::runtime_error przekazujemy wiadomość, która będzie wyświetlana w przypadku wystąpienia wyjątku
class zly_ruch: public runtime_error{
    public:
    zly_ruch(const string& message): runtime_error(message){}
};

class poza_zakres: public runtime_error{
    public:
    poza_zakres(const string& message): runtime_error(message){}
};

class Plansza{
    public:
    vector<int> plansza;

    //inicjalizacja w konstruktorze jest bardziej elastyczna,
    //łatwiejsza modyfikacja i dodawanie danych dynamicznych
    Plansza(){plansza.resize(9,0);}

    //rysujemy planszę do gry
    void rysuj(){
        cout<<"\n";
        //size_t jest bezpiecznym typem dla rozmiarów i indeksów tablic oraz kontenerów
        //nie trzeba korzystać z static_cast<int> dla plansza.size()
        for(size_t indeks=0; indeks<plansza.size();indeks++){
        //for (int indeks = 0; indeks < static_cast<int>(plansza.size()); indeks++){
            int wartosc=plansza[indeks];

            if(wartosc==0){
                cout<<indeks;
            }else{
                if(wartosc==1){
                    cout<<'X';
                }else{
                    cout<<'O';
                }
            }

            if(indeks%3 != 2){
                cout<<"|";
            } else if(indeks!=8){
                cout<<"\n---------\n";
            }
        }
        cout<<"\n\n";
    }

    //sprawdzamy czy plansza jest w całości zapełniona - czy nie zawiera żadnego 0
    bool czy_pelne(){
        //none_of zwraca true, jeśli żaden element w podanym zakresie nie ma wartości 0
        //jeśli jest choć jedna wartość 0 to zwróci false
        return none_of(plansza.begin(),plansza.end(),[](int war){return war==0;});
    }

    bool sprawdz_wygrana(int gracz){
        int poz_wygrywajace[8][3]={
            {0,1,2},{3,4,5},{6,7,8},//poziome
            {0,3,6},{1,4,7},{2,5,8},//pionowe
            {0,4,8},{2,4,6}//skos
        };

        //sprawdzamy czy któraś z konfiguracji wygrywających jest zajęta przez 1 z graczy
        for(auto& poz: poz_wygrywajace){
            if(plansza[poz[0]]==gracz && plansza[poz[1]]==gracz && plansza[poz[2]]==gracz){
                    return true;
            }
        }
        return false;
    }

    //wypełniamy tablice aktulanie wykonanym ruchem
    void aktualizuj(int ruch, int symbol){
        plansza[ruch]=symbol;
    }
};

//klasa bazowa dla graczy
class gracz{
    public:
    virtual int podaj_ruch(const vector<int>& plansza)=0;
    virtual char podaj_symbol() const =0;
};

//klasa gracza ludzkiego
class gracz_czlowiek: public gracz{
    public:
    char symbol;

    gracz_czlowiek(char s): symbol(s){}

    int podaj_ruch(const vector<int>& plansza) override {
        int ruch;
        cout<<"Wybierz numer pola z zakresu (0-8): ";
        cin>>ruch;

        //sprawdzamy, czy nie zostało podane pole poza zakresem
        if(ruch<0 || ruch>8){
            throw poza_zakres("Wybor spoza zakresu! Wybierz pole w zakresie 0-8.");
        }

        //sprawdzamy, czy pole nie jest już zajęte
        while(plansza[ruch]!=0){
            throw zly_ruch("To pole jest juz zajete! Wybierz inne.");
        }

        return ruch;
    }

    //zwracamy symbol gracza
    char podaj_symbol() const override {
        return symbol;
    }
};

class gracz_komputer: public gracz{
    private:
    //komputer wykonuje ruch wygrywający
    int wygrana(const vector<int>& plansza){
        int wygrywajace[8][3]={
            {0,1,2},{3,4,5},{6,7,8},//poziome
            {0,3,6},{1,4,7},{2,5,8},//pionowe
            {0,4,8},{2,4,6}//skos
        };

        for(auto& poz: wygrywajace){
            int zajete_komputer=0;//ile pól zajętych przez komputer
            int puste=-1;//pole puste - możliwe do zajęcia

            //sprawdzamy czy jest takie pole, dzięki któremu komputer może wygrać
            for(int indeks:poz){
                if(plansza[indeks]==2){zajete_komputer++;}
                else if(plansza[indeks]==0){puste=indeks;}
            }

            if(zajete_komputer==2 && puste !=-1){
                return puste;//wybór pola wygrywającego przez komputer
            }
        }
        return -1;
    }

    //komputer wykonuje ruch blokujący gracza
    int blokuj(const vector<int>& plansza){
        int wygrywajace[8][3]={
            {0,1,2},{3,4,5},{6,7,8},//poziome
            {0,3,6},{1,4,7},{2,5,8},//pionowe
            {0,4,8},{2,4,6}//skos
        };

        for(auto& poz: wygrywajace){
            int zajete_gracz=0;//ile pól zajętych przez gracza
            int puste=-1;//pole puste - możliwe do zajęcia

            //sprawdzamy czy jest takie pole, dzięki któremu gracz wygra
            for(int indeks:poz){
                if(plansza[indeks]==1){zajete_gracz++;}
                else if(plansza[indeks]==0){puste=indeks;}
            }

            if(zajete_gracz==2 && puste !=-1){
                return puste;//wybieramy to pole, aby zablokować gracza
            }
        }
        return -1;
    }

    //komputer wykonuje losowy ruch
    int losowe(const vector<int>& plansza){
        vector<int> mozliwe_ruchy;

        //wypełniamy wektor mozliwe_ruchy numerami pól, które są wolne
        for(size_t i=0;i<plansza.size();i++){
        //for(int i = 0; i < static_cast<int>(plansza.size()); i++){
            if(plansza[i]==0){
                mozliwe_ruchy.push_back(i);
            }
        }

        //wybieramy losowy ruch komputera za pomocą funkcji rand() i reszty z dzielenia przez rozmiar wektora mozliwe_ruchy
        int losowy_ruch=mozliwe_ruchy[rand()%mozliwe_ruchy.size()];
        return losowy_ruch;
    }

    public:
    char symbol;

    gracz_komputer(char s): symbol(s){}

    //komputer sprawdza czy może wygrać, zablokować gracza, jeśli nie to ruch jest losowy
    int podaj_ruch(const vector<int>& plansza) override {
        int ruch = wygrana(plansza);
        if(ruch != -1){
            return ruch;
        }

        ruch=blokuj(plansza);
        if(ruch != -1){
            return ruch;
        }

        ruch=losowe(plansza);
        return ruch;
    }

    char podaj_symbol() const override{
        return symbol;
    }
};

// Funkcja zatrzymująca program po błędzie
void waitForKeyPress() {
    std::cout << "Naciśnij dowolny klawisz, aby kontynuować...";
    std::cin.ignore(); // Ignoruje poprzednią linię wejścia
    std::cin.get();    // Czeka na naciśnięcie dowolnego klawisza
}

//funkcja odpowiedzialna za działanie gry
void graj(Plansza& plansza_do_gry, gracz* obecny_gracz, gracz_czlowiek& gracz1, gracz_komputer& gracz2){
    int zwyciezca=0;

    while(plansza_do_gry.czy_pelne()==false && zwyciezca==0){
        clearScreen();
        plansza_do_gry.rysuj();

        try{
            int ruch=obecny_gracz->podaj_ruch(plansza_do_gry.plansza);//podajemy ruch

            int symbol;
            if(obecny_gracz->podaj_symbol()=='X'){
                symbol=1;
            }else{
                symbol=2;
            }

            //aktualizuje plansze
            plansza_do_gry.aktualizuj(ruch, symbol);

            //sprawdzamy czy jest zwycięzca
            if(plansza_do_gry.sprawdz_wygrana(symbol)){
                zwyciezca=symbol;
            }
        }
        catch(const exception& e)
        {
            cout<<e.what()<<endl;
            waitForKeyPress();
            continue;
        }
        
        //sprawdzamy, czy obecny_gracz jest wskaźnikiem do gracz1
        if(obecny_gracz==&gracz1){
            //jeśli tak, przypisujemy do obecny_gracz wskaźnik na gracz2
            obecny_gracz=(gracz*)&gracz2;
        }else{
            //jeśli nie, przypisujemy do obecny_gracz wskaźnik na gracz1
            obecny_gracz=(gracz*)&gracz1;
        }
    }

    clearScreen();
    plansza_do_gry.rysuj();

    //wyswietlamy informację kto wygrał
    if(zwyciezca==1){
        cout<<"Gratulacje! Wygrales!\n";
    }else if(zwyciezca==2){
        cout<<"Komputer wygral!\n";
    }else{
        cout<<"Remis!\n";
    }
}

//wybieramy kto zaczyna gre, jeśli człowiek, wtedy zwracany jest wskaźnik na gracz1, jeśli komputer to na gracz2
gracz* kto_zaczyna(gracz_czlowiek& gracz1, gracz_komputer& gracz2){
    char wybor;
    cout<<"Wybierz, kto zaczyna: (g)racz czy (k)omputer: ";
    cin>>wybor;

    if(wybor=='g'){
        return &gracz1;
    }else{
    return &gracz2;
    }
}


int main(){
    srand(static_cast<unsigned int>(time(0)));//inicjuje generator liczb losowych

    //tworzymy plansze i graczy
    Plansza plansza_do_gry;
    gracz_czlowiek czlowiek('X');
    gracz_komputer komputer('O');

    gracz* obecny_gracz = kto_zaczyna(czlowiek,komputer);//wybieramy kto zaczyna gre
    graj(plansza_do_gry,obecny_gracz,czlowiek,komputer);//odpalamy gre

    return 0;
}
