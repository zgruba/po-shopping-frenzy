#include <iostream>
#include <math.h>
#include <string>
#include <unordered_map>
#include <iterator>
#include <vector>
#include <unordered_set>
#include <memory>
#include <sstream>

using namespace std;

// struktura reprezentująca produkty w sklepach
struct Produkt {
    string nazwa;
    Produkt() = delete;
    Produkt(string& _nazwa): nazwa(_nazwa) {};
    Produkt(const Produkt&) = delete;
    Produkt& operator=(const Produkt&) = delete;
    Produkt(Produkt&&) = default;
    ~Produkt() = default;
};

ostream& operator<<(ostream &wyjscie, const Produkt &p) {
    wyjscie << p.nazwa << endl;
    return wyjscie;
}

typedef double cena_t;
typedef vector<unique_ptr<Produkt>> sklad_t;
typedef pair<cena_t, sklad_t> towary_t;
typedef pair<cena_t, size_t> towar_info_t;
typedef unordered_map<string, towary_t> inwentarz_t;
typedef pair <double, double> lokalizacja_t;
typedef double odleglosc_t;

// funkcje pomocnicze
odleglosc_t odleglosc(lokalizacja_t l1, lokalizacja_t l2) {
    return sqrt(pow(l1.first - l2.first, 2) + pow(l1.second - l2.second, 2));
}

cena_t cena(const towary_t& ti) {    
    return ti.first;
}

cena_t cena(towar_info_t ti) {
    return ti.first;
}

size_t ilosc(const towary_t& ti) {
    return ti.second.size();
}

size_t ilosc(towar_info_t ti) {
    return ti.second;
}

class Miasto;

// klasa abstrakcyjna reprezentująca obiekty handlowe
class JednostkaHandlowa {
protected:
    Miasto* miastoRejestracji = nullptr;
    string nazwa;
    bool moznaTrzymacWMiescie = true;
public:
    JednostkaHandlowa(string& _nazwa) : nazwa(_nazwa) {};
    JednostkaHandlowa(const JednostkaHandlowa&) = delete;
    JednostkaHandlowa& operator=(const JednostkaHandlowa&) = delete;
    JednostkaHandlowa(JednostkaHandlowa&&) = default;
    virtual ~JednostkaHandlowa() = default;    
    virtual unique_ptr<Produkt> sprzedaj(const string& nazwa) = 0;
    virtual bool czyMaTowar(const string& nazwa) const = 0;
    virtual double odlegloscOd(const lokalizacja_t l) const { return -1; };
    virtual string doStringa() const = 0;
    virtual towar_info_t daneProduktu(const string& nazwa) const = 0;
    void zarejestruj(Miasto* m) { miastoRejestracji = m; };
    bool czyMoznaTrzymacWMiescie() const { return moznaTrzymacWMiescie; };
    string getNazwa() { return nazwa; };
    Miasto* getMiastoRejestracji() { return miastoRejestracji; };
};

ostream& operator<<(ostream &wyjscie, const JednostkaHandlowa &jh) {
    return wyjscie << jh.doStringa();
}

class StalyKlient;

// klasa abstrakcyjna reprezentująca sklep
class Sklep : public JednostkaHandlowa {
protected:
    inwentarz_t i;
    unordered_set<StalyKlient*> staliKlienci;
public:
    Sklep() = delete;
    Sklep(string& _nazwa, inwentarz_t&& _i) : JednostkaHandlowa(_nazwa), i(move(_i)) {};
    Sklep(const Sklep&) = delete;
    Sklep& operator=(const Sklep&) = delete;
    Sklep(Sklep&&) = default;
    virtual ~Sklep() = 0;
    unique_ptr<Produkt> sprzedaj(const string& nazwa) override;
    bool czyMaTowar(const string& nazwa) const override;
    towar_info_t daneProduktu(const string& nazwa) const override;
    vector<string> przegladAsortymentu() const;
    string doStringa() const override;
    unordered_set<StalyKlient*> getStaliKlienci() { return staliKlienci; };
    void addToStaliKlienci(StalyKlient* sk);
    void usunStalegoKlienta(StalyKlient* sk);
};

ostream& operator<<(ostream &wyjscie, const Sklep &s) {
    return wyjscie << s.doStringa();
}

// metoda zwracająca unikalny wskaźnik na sprzedawany produkt 
unique_ptr<Produkt> Sklep::sprzedaj(const string& nazwa) {
    if (czyMaTowar(nazwa)) {
        towary_t& produkt = i.find(nazwa)->second;
        sklad_t& sklad = produkt.second;
        auto p = move(sklad.back());
        sklad.pop_back();
        return p;
    }
    return nullptr;
}

// metoda sprawdzająca czy dany towar jest w sklepie
bool Sklep::czyMaTowar(const string& nazwa) const {
    return i.find(nazwa) != i.end() && ilosc(i.find(nazwa)->second) > 0;
}

// metoda zwracająca parę informacji o produkcie w sklepie
towar_info_t Sklep::daneProduktu(const string& nazwa) const {
    auto dane = &i.find(nazwa)->second;
    return {cena(*dane), ilosc(*dane)};
}

// metoda zwracająca vector z nazwami dostępnych produktów
vector<string> Sklep::przegladAsortymentu() const {
    vector<string> asortyment;
    for (const auto& towar : i) {
        asortyment.push_back(towar.first);
    }
    return asortyment;
}

// metoda wypisująca asortyment sklepu
string Sklep::doStringa() const {
    stringstream ss;
    ss << "\n";
    ss << "Asortyment sklepu: " << nazwa << "\n";
    for(const auto& towar : i){
        ss << "Produkt:"<< towar.first << " --> (cena, ilość): (" << towar.second.first <<", "<< towar.second.second.size() <<") \n";
    }
    ss << "\n";
    return ss.str();
}

void Sklep::addToStaliKlienci(StalyKlient* sk) {
    staliKlienci.insert(sk);
}

void Sklep::usunStalegoKlienta(StalyKlient* sk) {
    staliKlienci.erase(sk);
}

class Siec;
class Miasto;

// klasa reprezuentująca sklep zwykły - mający swoją lokalizacje
class SklepZwykly : public Sklep {
    friend class Siec;
    Siec* czyWSieci = nullptr;
    lokalizacja_t tuSklep; // dopiero sklep zwykly ma lokalizację, 
    //bo dopuszamy stworzenie klasy SklepInternetowy bez lokalizacji dziedziczący po sklepie
public:
    SklepZwykly() = delete;
    SklepZwykly(string _nazwa, inwentarz_t&& _i, lokalizacja_t _tu_sklep) : Sklep(_nazwa, move(_i)), tuSklep(_tu_sklep) {};
    SklepZwykly(const SklepZwykly&) = delete;
    SklepZwykly& operator=(const SklepZwykly&) = delete;
    SklepZwykly(SklepZwykly&&) = default;
    ~SklepZwykly();
    double odlegloscOd(lokalizacja_t l) const override;
    void zrezygnujZSieci();
    void powiadomOWejsciuSieci();
    void powiadomOWyjsciuSieci();
};
 
// metoda zwracająca odległość sklepu zwykłego od danej lokalizacji
double SklepZwykly::odlegloscOd(const lokalizacja_t l) const  {
    return odleglosc(tuSklep, l);
};

// klasa reprezentująca Siec
class Siec : public JednostkaHandlowa {
private: 
    unordered_set<shared_ptr<SklepZwykly>> listaSklepow;
    double marza;
public:
    Siec() = delete;
    Siec(string _nazwa, double _marża) : JednostkaHandlowa(_nazwa), marza(_marża) {};
    Siec(const Siec&) = delete;
    Siec& operator=(const Siec&) = delete;
    Siec(Siec&&) = default;
    ~Siec();
    void dodajSklep(shared_ptr<SklepZwykly> s);
    void usunSklep(shared_ptr<SklepZwykly> s);
    void usunSklepZListy(shared_ptr<SklepZwykly> s);
    void przejmijSiec(shared_ptr<Siec> s);
    bool czyMaTowar(const string& nazwa) const override;
    towar_info_t daneProduktu(const string& nazwa) const override;
    unique_ptr<Produkt> sprzedaj(const string& nazwa) override;
    string doStringa() const override;
    shared_ptr<SklepZwykly> znajdzSklep(SklepZwykly* s) const;
};

SklepZwykly::~SklepZwykly() {
    // poprawka
    if (czyWSieci != nullptr) {
        czyWSieci->usunSklepZListy(shared_ptr<SklepZwykly>(this));
    }
}

Siec::~Siec() {
    // poprawka
    for (auto sklep : listaSklepow) {
        sklep->czyWSieci = nullptr;
    }
}

ostream& operator<< (ostream &wyjscie, const Siec &s) {
    return wyjscie << s.doStringa();
}

// metoda zwracająca wskaźnik na sklep w sieci
shared_ptr<SklepZwykly> Siec::znajdzSklep(SklepZwykly* s) const {
    for (const shared_ptr<SklepZwykly>& sklep : listaSklepow) {
        if (sklep.get() == s) {
            return sklep;
        }
    }
    return nullptr;
}

// metoda realizująca rezygnację z bycia w sieci
void SklepZwykly::zrezygnujZSieci() {
    if (czyWSieci != nullptr) {
        czyWSieci->usunSklep(czyWSieci->znajdzSklep(this));
        czyWSieci = nullptr;
    }
}

// metoda sprawdzająca czy w sieci jest towar
bool Siec::czyMaTowar(const string& nazwa) const {
    for (const shared_ptr<SklepZwykly>& sklep : listaSklepow) {
        if (sklep->czyMaTowar(nazwa)) {
            return true;
        }
    }
    return false;
}

// metoda zwracająca parę informacji o produkcie
towar_info_t Siec::daneProduktu(const string& nazwa) const {
    for (const shared_ptr<SklepZwykly>& sklep : listaSklepow) {
        if (sklep->czyMaTowar(nazwa)) {
            return sklep->daneProduktu(nazwa);
        }
    }
    throw invalid_argument("Nie ma takiego produktu w sieci");
}

// metoda zracająca unikalny wskaźnik na produkt w ramach sprzedaży
unique_ptr<Produkt> Siec::sprzedaj(const string& nazwa) {
    for (const shared_ptr<SklepZwykly>& sklep : listaSklepow) {
        if (sklep->czyMaTowar(nazwa)) {
            return sklep->sprzedaj(nazwa);
        }
    }
    throw invalid_argument("Nie ma takiego produktu w sieci");
}

// metoda wypisująca sklepy w sieci i ich asortyment
string Siec::doStringa() const {
    stringstream ss;
    ss << "\n";
    ss << "Sklepy w sieci " << nazwa << ":\n";
    ss << "\n" << "Marża: " << marza << "\n";
    ss << "\n";
    for (const auto& sklep : listaSklepow){
        ss << sklep->getNazwa() << "\n";
        ss << sklep->doStringa();
    }
    ss << "--------\n";
    return ss.str();
}

class Kupujacy;

// klasa reprezentująca miasto
class Miasto {
private:
    unordered_set<shared_ptr<JednostkaHandlowa>> listaSklepow;
    vector<shared_ptr<Kupujacy>> listaMieszkancow;
public:
    Miasto() = default;
    Miasto(const Miasto&) = delete;
    Miasto& operator=(const Miasto&) = delete;
    Miasto(Miasto&&) = default;
    ~Miasto() = default;
    void symuluj();
    void dodajKupujacego(shared_ptr<Kupujacy> k);
    void dodajJednostkeHandlowa(shared_ptr<JednostkaHandlowa> jh);
    void usunJednostkeHandlowa(shared_ptr<JednostkaHandlowa> jh);
    const unordered_set<shared_ptr<JednostkaHandlowa>>& przegladajSklepy() const;

    friend ostream& operator<< (ostream &wyjscie, const Miasto &m);
};

// metoda realizująca dodawanie mieszkańców do listy w mieście
void Miasto::dodajKupujacego(shared_ptr<Kupujacy> k){
    listaMieszkancow.push_back(k);
}

// metoda realizująca dodawanie obiektów handlowych do listy w mieście
void Miasto::dodajJednostkeHandlowa(shared_ptr<JednostkaHandlowa> jh){
   if (jh->czyMoznaTrzymacWMiescie()) {
        listaSklepow.insert(jh);
        jh->zarejestruj(this);
    }
    else {
        throw invalid_argument("Nie można trzymać tej jednostki handlowej w mieście");
    }
}

// metoda realizująca usuwanie obiektów handlowych z listy w mieście
void Miasto::usunJednostkeHandlowa(shared_ptr<JednostkaHandlowa> jh){
    listaSklepow.erase(jh);
}

// metoda zwracająca kontener ze wskaźnikami na sklepy w sieci
const unordered_set<shared_ptr<JednostkaHandlowa>>& Miasto::przegladajSklepy() const {
    return listaSklepow;
}

// metoda realizująca dodawanie sklepów do sieci
void Siec::dodajSklep(shared_ptr<SklepZwykly> s) {
    if (s->czyWSieci != nullptr) {
        throw invalid_argument("Ten sklep już jest w sieci");
    }
    s->czyWSieci = this;
    s->moznaTrzymacWMiescie = false;
    listaSklepow.insert(s);
    s->powiadomOWejsciuSieci();
    s->moznaTrzymacWMiescie = false;
    if (s->getMiastoRejestracji() != nullptr)
        s->getMiastoRejestracji()->usunJednostkeHandlowa(s);
}

// metoda // nie kupi prezentu bo ulubiony sklep w siecirealizująca usuwanie sklepów z sieci
void Siec::usunSklep(shared_ptr<SklepZwykly> s) {
    if (this != s->czyWSieci) {
        throw invalid_argument("Ten sklep nie jest w tej sieci");
    }
    listaSklepow.erase(s);
    s->czyWSieci = nullptr;
    s->moznaTrzymacWMiescie = true;
    miastoRejestracji->dodajJednostkeHandlowa(s);
    s->powiadomOWyjsciuSieci();
}

void Siec::usunSklepZListy(shared_ptr<SklepZwykly> s) {
    listaSklepow.erase(s);
}

// metoda realizująca przejęcie innej sieci
void Siec::przejmijSiec(shared_ptr<Siec> s) {
    listaSklepow.insert(s->listaSklepow.begin(), s->listaSklepow.end());
    for (auto& sklep : s->listaSklepow) {
        sklep->czyWSieci = this;
    }
    if (miastoRejestracji != nullptr){
        miastoRejestracji->usunJednostkeHandlowa(s);
    }
    s->listaSklepow.clear();
}

// klasa abstrakcyjna reprezentująca kupujących
class Kupujacy {
protected:
    string imie;
    string nazwisko;
    double budzet;
    lokalizacja_t tuJestem;
    const Miasto* mojeMiasto;
    bool kupiony = false;
    unique_ptr<Produkt> kupionyPrezent = nullptr;
public:
    Kupujacy() = delete;
    Kupujacy(string _imie, string _nazwisko, double _budzet, lokalizacja_t _tu, const Miasto* _miasto) : 
        imie(_imie), nazwisko(_nazwisko), budzet(_budzet), tuJestem(_tu), mojeMiasto(_miasto) {};
    Kupujacy(const Kupujacy&) = delete;
    Kupujacy& operator=(const Kupujacy&) = delete;
    Kupujacy(Kupujacy&&) = default;
    virtual ~Kupujacy() = default;
    bool czyKupiony() const { return kupiony; };
    virtual void kupPrezent() = 0;
    virtual string przedstawSie() const = 0;
};

ostream& operator<<(ostream &wyjscie, const Kupujacy &k) {
    return wyjscie << k.przedstawSie();
}

// metoda realizująca zakupy oraz wypisywanie stanu miasta przed i po
void Miasto::symuluj() {
    cout << *this;
    for(const auto& klient : listaMieszkancow){
        cout << "\n";
        klient->kupPrezent();
    }
    cout << *this;
}

ostream& operator<< (ostream &wyjscie, const Miasto &m) {
    wyjscie << "\n";
    wyjscie << "Mieszkańcy:";
    wyjscie << "\n";
    for(const auto& klient : m.listaMieszkancow){
        wyjscie << klient->przedstawSie();
    }

    wyjscie << "Stan sklepów w mieście:" << "\n";
    for(auto sklep : m.listaSklepow){
        wyjscie << sklep->doStringa();
    }
    return wyjscie;
}

// klasa reprezentująca klientów posiadających swój ulubiony sklep
class StalyKlient : public Kupujacy {
protected:
    shared_ptr<Sklep> ulubionySklep;
    bool czyUlubionyDostepny;
public:
    StalyKlient() = delete;
    StalyKlient(string _imie, string _nazwisko, double _budzet, lokalizacja_t _tu, const Miasto* _miasto, shared_ptr<Sklep> _ulubiony)
        : Kupujacy(_imie, _nazwisko, _budzet, _tu, _miasto), ulubionySklep(_ulubiony) { 
            ulubionySklep->addToStaliKlienci(this);
            if(ulubionySklep->getMiastoRejestracji() == mojeMiasto && ulubionySklep->czyMoznaTrzymacWMiescie()) {
                czyUlubionyDostepny = true;} else {
                    czyUlubionyDostepny = false;
                }
            };
    StalyKlient(const StalyKlient&) = delete;
    StalyKlient& operator=(const StalyKlient&) = delete;
    StalyKlient(StalyKlient&&) = default;
    virtual ~StalyKlient();
    string przedstawSie() const override;
    void usunUlubionySklep();
    
    friend void SklepZwykly::powiadomOWejsciuSieci();
    friend void SklepZwykly::powiadomOWyjsciuSieci();
};

Sklep::~Sklep() {
    // poprawka
    for (auto sk : staliKlienci) {
        sk->usunUlubionySklep();
    }
}

StalyKlient::~StalyKlient() {
    // poprawka
    if (ulubionySklep != nullptr) {
        ulubionySklep->usunStalegoKlienta(this);
    }
}

ostream& operator<<(ostream &wyjscie, const StalyKlient &k) {
    return wyjscie << k.przedstawSie();
}

// metoda która powiadamia, że sklep wszedł do sieci
void SklepZwykly::powiadomOWejsciuSieci() {
    for (auto& klient : staliKlienci) {
        klient->czyUlubionyDostepny = false;
    }
}

// metoda która powiadamia, że sklep wyszedł z sieci
void SklepZwykly::powiadomOWyjsciuSieci() {
    for (auto& klient : staliKlienci) {
        if (miastoRejestracji == klient->mojeMiasto) {
            klient->czyUlubionyDostepny = true;
        }
    }
}

// metoda przedstawiająca kupujących
string StalyKlient::przedstawSie() const {
    stringstream ss;
    ss << "\n";
    ss << "Czesc, jestem " << imie << " "<<nazwisko<< "\n";
    ss << "Chcę kupić w " << ulubionySklep->getNazwa() << "\n";
    ss << "Mój budżet to " << budzet << "\n";
    ss << "Czy ulubiony dostepny " << czyUlubionyDostepny << "\n";
    if(kupiony == true){
        ss << "Kupiłem " << *kupionyPrezent << "\n";
    } else {
        ss << "Nie kupiłem prezentu! \n";
    }
    ss << "\n";
    return ss.str();
}

void StalyKlient::usunUlubionySklep() {
    czyUlubionyDostepny = false;
    ulubionySklep = nullptr;
}

// klasa reprezentująca stałego klienta, który szuka najdroższego prezentu
class MaksymalistaCenowy : public StalyKlient {
public:
    MaksymalistaCenowy() = delete;
    MaksymalistaCenowy(string _imie, string _nazwisko, double _budzet, lokalizacja_t _tu, const Miasto* _miasto, shared_ptr<Sklep> _ulubiony)
        : StalyKlient(_imie, _nazwisko, _budzet, _tu, _miasto, _ulubiony) {};
    MaksymalistaCenowy(const MaksymalistaCenowy&) = delete;
    MaksymalistaCenowy& operator=(const MaksymalistaCenowy&) = delete;
    MaksymalistaCenowy(MaksymalistaCenowy&&) = default;
    ~MaksymalistaCenowy() = default;
    void kupPrezent();
};

// metoda realizująca zakup przez Maksymalistę
void MaksymalistaCenowy::kupPrezent() {
    double aktualna = -1;
    string kandydat;
    vector<string> wSklepie = ulubionySklep->przegladAsortymentu();
    cout << "MaksymalistaCenowy "<< imie << " "<<nazwisko<< " kupuje" <<"\n";
    for (const string& nazwa : wSklepie) {
        towar_info_t produkt = ulubionySklep->daneProduktu(nazwa);
        if (cena(produkt) > aktualna && cena(produkt) <= budzet && ilosc(produkt) > 0) {
            aktualna = cena(produkt);
            kandydat = nazwa;
        }
    }
    if (aktualna != -1 && czyUlubionyDostepny) {
        cout << "Udało się kupić prezent "<< ulubionySklep->getNazwa()<<"!"<<"\n";
        budzet = budzet - aktualna;
        kupionyPrezent = ulubionySklep->sprzedaj(kandydat);
        kupiony = true;
    } else {
        cout<<"Nie udało się kupić prezentu"<<"\n";
    }
}

// klasa reprezentująca stałego klienta, który szuka najtańszego prezentu
class MinimalistaCenowy : public StalyKlient {
public:
    MinimalistaCenowy() = delete;
    MinimalistaCenowy(string _imie, string _nazwisko, double _budzet, lokalizacja_t _tu, const Miasto* _miasto, shared_ptr<Sklep> _ulubiony)
        : StalyKlient(_imie, _nazwisko, _budzet, _tu, _miasto, _ulubiony) {};
    MinimalistaCenowy(const MinimalistaCenowy&) = delete;
    MinimalistaCenowy& operator=(const MinimalistaCenowy&) = delete;
    MinimalistaCenowy(MinimalistaCenowy&&) = default;
    ~MinimalistaCenowy() = default;
    void kupPrezent();
};

// metoda realizująca zakup przez Minimalistę
void MinimalistaCenowy::kupPrezent() {
    double aktualna = -1;
    string kandydat;
    vector<string> wSklepie = ulubionySklep->przegladAsortymentu();
    cout << "MinimalistaCenowy "<< imie << " "<<nazwisko<< " kupuje" <<"\n";
    for (const string& nazwa : wSklepie) {
        towar_info_t produkt = ulubionySklep->daneProduktu(nazwa);
        if ((aktualna == -1 || cena(produkt) < aktualna) && cena(produkt) <= budzet && ilosc(produkt) > 0 ) {
            aktualna = cena(produkt);
            kandydat = nazwa;
        }
    }
    if (aktualna != -1 && czyUlubionyDostepny) {
        cout << "Udało się kupić prezent w "<< ulubionySklep->getNazwa()<<"!"<<"\n";
        budzet = budzet - aktualna;
        kupionyPrezent = ulubionySklep->sprzedaj(kandydat);
        kupiony = true;
    }  else {
        cout<<"Nie udało się kupić prezentu"<<"\n";
    }
}

// klasa reprezentująca stałego klienta, który szuka jakiegoś prezentu losowo
// w ulubionym sklepie
class LosowyKlient : public StalyKlient {
private:
    const int doTyluRazySztuka = 3;
public:
    LosowyKlient() = delete;
    LosowyKlient(string _imie, string _nazwisko, double _budzet, lokalizacja_t _tu, const Miasto* _miasto, shared_ptr<Sklep> _ulubiony)
        : StalyKlient(_imie, _nazwisko, _budzet, _tu, _miasto, _ulubiony) {};
    LosowyKlient(const LosowyKlient&) = delete;
    LosowyKlient& operator=(const LosowyKlient&) = delete;
    LosowyKlient(LosowyKlient&&) = default;
    ~LosowyKlient() = default;
    void kupPrezent();
};

// metoda realizująca zakup przez LosowegoKlienta
void LosowyKlient::kupPrezent() {
    vector<string> wSklepie = ulubionySklep->przegladAsortymentu();
    cout << "LosowyKlient "<< imie << " "<<nazwisko<< " kupuje" <<"\n";
    int licznik = 0;
    for (int i = 0; i < doTyluRazySztuka; i++) {
        vector<string>::iterator random_it = wSklepie.begin();
        advance(random_it, rand() % wSklepie.size());
        towar_info_t produkt = ulubionySklep->daneProduktu(*random_it);
        licznik++; 
        if (cena(produkt) <= budzet && ilosc(produkt) > 0 && czyUlubionyDostepny){
            cout << "Udało się kupić prezent "<< ulubionySklep->getNazwa()<<"!"<<"\n";
            budzet = budzet - cena(produkt);
            kupionyPrezent = ulubionySklep->sprzedaj(*random_it);
            kupiony = true;
            return;
        }
    }
    cout << "Nie udało się kupić prezentu\n";
}

// klasa reprezentująca klientów szukających kontretnego prezentu
class PoszukujacyKlient : public Kupujacy {
protected:
    string wymarzonyPrezent;
public:
    PoszukujacyKlient() = delete;
    PoszukujacyKlient(string _imie, string _nazwisko, double _budzet, lokalizacja_t _tu, const Miasto* _miasto, string _wymarzony)
        : Kupujacy(_imie, _nazwisko, _budzet, _tu, _miasto), wymarzonyPrezent(_wymarzony) {};
    PoszukujacyKlient(const PoszukujacyKlient&) = delete;
    PoszukujacyKlient& operator=(const PoszukujacyKlient&) = delete;
    PoszukujacyKlient(PoszukujacyKlient&&) = default;
    virtual ~PoszukujacyKlient() = default;
    string przedstawSie() const override;
};

ostream& operator<<(ostream& os, const PoszukujacyKlient& klient) {
    return os << klient.przedstawSie();
}

// metoda przedstawiająca kupujących
string PoszukujacyKlient::przedstawSie() const {
    stringstream ss;
    ss << "\n";
    ss << "Czesc, jestem " << imie << " "<<nazwisko<< "\n";
    ss << "Chcę kupić " << wymarzonyPrezent << "\n";
    ss << "Mój budżet to " << budzet << "\n";
    if(kupiony == true){
        ss << "Kupiłem " <<*kupionyPrezent <<"\n";
    } else {
        ss << "Nie kupiłem prezentu! \n";
    }
    ss << "\n";
    return ss.str();
}

// klasa reprezentująca klienta, który szuka wymarzonego prezentu w najlepszej cenie
class OszczednyKlient : public PoszukujacyKlient {
public:
    OszczednyKlient() = delete;
    OszczednyKlient(string _imie, string _nazwisko, double _budzet, lokalizacja_t _tu, const Miasto* _miasto, string _wymarzony)
        : PoszukujacyKlient(_imie, _nazwisko, _budzet, _tu, _miasto, _wymarzony) {};
    OszczednyKlient(const OszczednyKlient&) = delete;
    OszczednyKlient& operator=(const OszczednyKlient&) = delete;
    OszczednyKlient(OszczednyKlient&&) = default;
    ~OszczednyKlient() = default;
    void kupPrezent();
};

// metoda realizująca zakup przez OszczednegoKlienta
void OszczednyKlient::kupPrezent() {
    shared_ptr<JednostkaHandlowa> kandydat = nullptr;
    double najcena = budzet + 1;
    unordered_set<shared_ptr<JednostkaHandlowa>> lista = mojeMiasto->przegladajSklepy();
    cout << "OszczednyKlient "<< imie << " "<<nazwisko<< " kupuje" <<"\n";
    for (auto sklep : lista) {
        if (sklep->czyMaTowar(wymarzonyPrezent)) {
            towar_info_t dane = sklep->daneProduktu(wymarzonyPrezent);
            if (cena(dane) < najcena){
                kandydat = sklep;
                najcena = cena(dane);
            }
        }   
    }
    if (kandydat != nullptr) {
        cout << "Udało się kupić prezent w " << kandydat->getNazwa() << "!"<<"\n";
        budzet = budzet - najcena;
        kupionyPrezent = kandydat->sprzedaj(wymarzonyPrezent);
        kupiony = true;
    } else {
        cout<<"Nie udało się kupić prezentu"<<"\n";
    }
}

// klasa reprezentująca klienta, który szuka wymarzonego prezentu w sklepach zwykłych
class TradycjonalistaKlient : public PoszukujacyKlient {
public:
    TradycjonalistaKlient() = delete;
    TradycjonalistaKlient(string _imie, string _nazwisko, double _budzet, lokalizacja_t _tu, const Miasto* _miasto, string _wymarzony)
        : PoszukujacyKlient(_imie, _nazwisko, _budzet, _tu, _miasto, _wymarzony) {};
    TradycjonalistaKlient(const TradycjonalistaKlient&) = delete;
    TradycjonalistaKlient& operator=(const TradycjonalistaKlient&) = delete;
    TradycjonalistaKlient(TradycjonalistaKlient&&) = default;
    ~TradycjonalistaKlient() = default;
    void kupPrezent();
};

// metoda realizująca zakup przez Tradycjonalistę
void TradycjonalistaKlient::kupPrezent() {
    shared_ptr<JednostkaHandlowa> kandydat = nullptr;
    double najodleglosc = -1, najcena = budzet + 1;
    cout << "TradycjonalistaKlient " << imie << " " << nazwisko << " kupuje" << "\n";
    for (auto sklep : mojeMiasto->przegladajSklepy()) {
        odleglosc_t jak_daleko = sklep->odlegloscOd(tuJestem);
        if (jak_daleko == -1 || !sklep->czyMaTowar(wymarzonyPrezent)) {
            continue;
        }
        towar_info_t dane = sklep->daneProduktu(wymarzonyPrezent);
        if ((najodleglosc == -1 || jak_daleko < najodleglosc) && ilosc(dane) > 0 && cena(dane) <= budzet) {
            kandydat = sklep;
            najodleglosc = jak_daleko;
            najcena = cena(dane);
        }
    }
    if (kandydat != nullptr) {
        cout << "Udało się kupić prezent w "<< kandydat->getNazwa()<<"!"<<"\n";
        budzet = budzet - najcena;
        kupionyPrezent = kandydat->sprzedaj(wymarzonyPrezent);
        kupiony = true;
    } else {
        cout << "Nie udało się kupić prezentu" << "\n";
    }
}

int main() {    
    // ------------------ tworzenie miasta -------------------
    Miasto Tragicz{};

    // ------------------ tworzenie i dodawanie sklepów do miasta -------------------
    inwentarz_t inw1;
    vector<double> veccena1 = {25.5, 4.99, 33.3, 12.4};
    vector<string> vecnazwa = {"pluszak", "piłka", "lalka", "torba"};
    vector<size_t> vecile1 = {6, 10, 4 , 25};
    for (int i = 0; i < 4; ++i) {
        double cena = veccena1[i];
        string nazwa = vecnazwa[i];
        vector<unique_ptr<Produkt>> v;
        for (size_t it = 0; it < vecile1[i]; ++it) {
            v.emplace_back(new Produkt(nazwa));
        }
        inw1[nazwa] = {cena, move(v)};
    }
    shared_ptr<SklepZwykly> sz1 = shared_ptr<SklepZwykly>(new SklepZwykly("Tutu", move(inw1), {2,6}));
    Tragicz.dodajJednostkeHandlowa(sz1);

    inwentarz_t inw2;
    vector<double> veccena2 = {14.5, 5.99, 35.3, 1.7};
    vector<size_t> vecile2 = {7, 14, 4 , 20};
    for (int i = 0; i < 4; ++i) {
        double cena = veccena2[i];
        string nazwa = vecnazwa[i];
        vector<unique_ptr<Produkt>> v;
        for (size_t it =0; it< vecile2[i]; ++it) {
            v.emplace_back(new Produkt(nazwa));
        }
        inw2[nazwa] = {cena, move(v)};
    }
    shared_ptr<SklepZwykly> sz2 = shared_ptr<SklepZwykly>(new SklepZwykly("Dudu", move(inw2), {3,4}));
    Tragicz.dodajJednostkeHandlowa(sz2);

    inwentarz_t inw3;
    vector<double> veccena3 = {23.5, 7.99, 37.3, 15.4};
    vector<size_t> vecile3 = {7, 13, 4 , 0};
    for (int i = 0; i<4; ++i) {
        double cena = veccena3[i];
        string nazwa = vecnazwa[i];
        vector<unique_ptr<Produkt>> v;
        for (size_t it =0; it< vecile3[i]; ++it) {
            v.emplace_back(new Produkt(nazwa));
        }
        inw3[nazwa] = {cena, move(v)};
    }
    shared_ptr<SklepZwykly> sz3 = shared_ptr<SklepZwykly>(new SklepZwykly("Mumu", move(inw3), {4,4}));
    Tragicz.dodajJednostkeHandlowa(sz3);

    inwentarz_t inw4;
    vector<double> veccena4 = {23.5, 9.99, 32.3, 11.4};
    vector<size_t> vecile4 = {8, 11, 4 , 23};
    for (int i = 0; i < 4; ++i) {
        double cena = veccena4[i];
        string nazwa = vecnazwa[i];
        vector<unique_ptr<Produkt>> v;
        for (size_t it =0; it< vecile4[i]; ++it) {
            v.emplace_back(new Produkt(nazwa));
        }
        inw4[nazwa] = {cena, move(v)};
    }
    shared_ptr<SklepZwykly> sz4 = shared_ptr<SklepZwykly>(new SklepZwykly("Zuzu", move(inw4), {9,1}));
    Tragicz.dodajJednostkeHandlowa(sz4);


    inwentarz_t inw5;
    vector<double> veccena5 = {26.5, 10.99, 34.7, 12.5};
    vector<size_t> vecile5 = {3, 1, 40, 5};
    for (int i = 0; i < 4; ++i) {
        double cena = veccena5[i];
        string nazwa = vecnazwa[i];
        vector<unique_ptr<Produkt>> v;
        for (size_t it =0; it< vecile5[i]; ++it) {
            v.emplace_back(new Produkt(nazwa));
        }
        inw5[nazwa] = {cena, move(v)};
    }
    shared_ptr<SklepZwykly> sz5 = shared_ptr<SklepZwykly>(new SklepZwykly("Koko", move(inw5), {7,-8}));
    Tragicz.dodajJednostkeHandlowa(sz5);

    inwentarz_t inw6;
    vector<double> veccena6 = {21.5, 15.99, 24.9, 11.5};
    vector<size_t> vecile6 = {0, 6, 42, 13};
    for (int i = 0; i < 4; ++i) {
        double cena = veccena6[i];
        string nazwa = vecnazwa[i];
        vector<unique_ptr<Produkt>> v;
        for (size_t it =0; it< vecile6[i]; ++it) {
            v.emplace_back(new Produkt(nazwa));
        }
        inw6[nazwa] = {cena, move(v)};
    }
    shared_ptr<SklepZwykly> sz6 = shared_ptr<SklepZwykly>(new SklepZwykly("Roko", move(inw6), {2,-5}));
    Tragicz.dodajJednostkeHandlowa(sz6);

    inwentarz_t inw7;
    vector<double> veccena7 = {26.5, 5.99, 19.9, 12.5};
    vector<size_t> vecile7 = {64, 3, 5, 15};
    for (int i = 0; i < 4; ++i) {
        double cena = veccena7[i];
        string nazwa = vecnazwa[i];
        vector<unique_ptr<Produkt>> v;
        for (size_t it =0; it< vecile7[i]; ++it) {
            v.emplace_back(new Produkt(nazwa));
        }
        inw7[nazwa] = {cena, move(v)};
    }

    shared_ptr<SklepZwykly> sz7 = shared_ptr<SklepZwykly>(new SklepZwykly("Smoko", move(inw7), {1,-1}));
    Tragicz.dodajJednostkeHandlowa(sz7);

    inwentarz_t inw8;
    vector<double> veccena8 = {13.5, 5.99, 21.9, 32.5};
    vector<size_t> vecile8 = {0, 3, 5, 1};
    for (int i = 0; i < 4; ++i) {
        double cena = veccena8[i];
        string nazwa = vecnazwa[i];
        vector<unique_ptr<Produkt>> v;
        for (size_t it =0; it< vecile8[i]; ++it) {
            v.emplace_back(new Produkt(nazwa));
        }
        inw8[nazwa] = {cena, move(v)};
    }

    shared_ptr<SklepZwykly> sz8 = shared_ptr<SklepZwykly>(new SklepZwykly("TipTop", move(inw8), {-5,-4}));
    Tragicz.dodajJednostkeHandlowa(sz8);

    inwentarz_t inw9;
    vector<double> veccena9 = {20.5, 10.99, 30.9, 50.5};
    vector<size_t> vecile9 = {1, 1, 1, 1};
    for (int i = 0; i < 4; ++i) {
        double cena = veccena9[i];
        string nazwa = vecnazwa[i];
        vector<unique_ptr<Produkt>> v;
        for (size_t it =0; it< vecile9[i]; ++it) {
            v.emplace_back(new Produkt(nazwa));
        }
        inw9[nazwa] = {cena, move(v)};
    }

    shared_ptr<SklepZwykly> sz9 = shared_ptr<SklepZwykly>(new SklepZwykly("TupTup", move(inw9), {0,0}));
    Tragicz.dodajJednostkeHandlowa(sz9);

    // ------------------ tworzenie i dodawanie sieci do miasta -------------------

    shared_ptr<Siec> Zabawki = shared_ptr<Siec>(new Siec("Stonka", 1.3));
    Tragicz.dodajJednostkeHandlowa(Zabawki);
    Zabawki->dodajSklep(sz1);
    Zabawki->dodajSklep(sz6);

    shared_ptr<Siec> Duperele = shared_ptr<Siec>(new Siec("Ślimak", 1.5));
    Tragicz.dodajJednostkeHandlowa(Duperele);
    Duperele->dodajSklep(sz2);
    Zabawki->dodajSklep(sz7);

    // ------------------ przejęcie sieci przez inną sieć -------------------

    Zabawki->przejmijSiec(Duperele);

    // ------------------ tworzenie i dodawanie kupujących do miasta -------------------
    
    shared_ptr<MaksymalistaCenowy> Maks = shared_ptr<MaksymalistaCenowy>(new MaksymalistaCenowy("Maks", "Malny", 39, {2,3}, &Tragicz, sz2));
    shared_ptr<MaksymalistaCenowy> Piotr = shared_ptr<MaksymalistaCenowy>(new MaksymalistaCenowy("Piotr", "Wielki", 100, {10,11}, &Tragicz, sz3));
    shared_ptr<MaksymalistaCenowy> Iwona = shared_ptr<MaksymalistaCenowy>(new MaksymalistaCenowy("Iwona", "Kowalska", 53, {5,-4}, &Tragicz, sz1));
    shared_ptr<MinimalistaCenowy> Minnie = shared_ptr<MinimalistaCenowy>(new MinimalistaCenowy("Minnie", "Malna", 15, {-1,3}, &Tragicz, sz4));
    shared_ptr<MinimalistaCenowy> Karol = shared_ptr<MinimalistaCenowy>(new MinimalistaCenowy("Karol", "Tyci", 23, {-2,6}, &Tragicz, sz3));
    shared_ptr<MinimalistaCenowy> Wisia = shared_ptr<MinimalistaCenowy>(new MinimalistaCenowy("Wisia", "Szym", 10, {2,-6}, &Tragicz, sz8));
    shared_ptr<LosowyKlient> Bart = shared_ptr<LosowyKlient>(new LosowyKlient("Bart", "Przypadkowy", 5, {5,5}, &Tragicz, sz1));
    shared_ptr<LosowyKlient> Tomek = shared_ptr<LosowyKlient>(new LosowyKlient("Tomek", "Zlotowka", 60, {0,0}, &Tragicz, sz5));
    shared_ptr<LosowyKlient> Pawel = shared_ptr<LosowyKlient>(new LosowyKlient("Pawel", "Gawel", 30, {2,2}, &Tragicz, sz9));
    shared_ptr<OszczednyKlient> Konrad = shared_ptr<OszczednyKlient>(new OszczednyKlient("Konrad", "Obiektowy", 20, {6,0}, &Tragicz, "torba"));
    shared_ptr<OszczednyKlient> Barbara = shared_ptr<OszczednyKlient>(new OszczednyKlient("Barbara", "Nowak", 13, {-2,7}, &Tragicz, "pluszak"));
    shared_ptr<OszczednyKlient> Bajtek = shared_ptr<OszczednyKlient>(new OszczednyKlient("Bajtek", "zBajtocji", 18, {-9,9}, &Tragicz, "piłka"));
    shared_ptr<TradycjonalistaKlient> Karyna = shared_ptr<TradycjonalistaKlient>(new TradycjonalistaKlient("Karyna", "Pompon", 40, {-3,1}, &Tragicz, "piłka"));
    shared_ptr<TradycjonalistaKlient> Jan = shared_ptr<TradycjonalistaKlient>(new TradycjonalistaKlient("Jan", "Szary", 20, {4,-3}, &Tragicz, "pluszak"));
    shared_ptr<TradycjonalistaKlient> Alicja = shared_ptr<TradycjonalistaKlient>(new TradycjonalistaKlient("Alicja", "Bitowa", 40, {1,-13}, &Tragicz, "lalka"));


    Tragicz.dodajKupujacego(Maks); // nie kupi prezentu bo ulubiony sklep w sieci
    Tragicz.dodajKupujacego(Bart); // nie kupi prezentu bo ulubiony sklep w sieci
    Tragicz.dodajKupujacego(Konrad);
    Tragicz.dodajKupujacego(Minnie);
    Tragicz.dodajKupujacego(Karyna);
    Tragicz.dodajKupujacego(Iwona); // nie kupi prezentu bo ulubiony sklep w sieci
    Tragicz.dodajKupujacego(Karol);
    Tragicz.dodajKupujacego(Tomek);
    Tragicz.dodajKupujacego(Barbara); // brak funduszy na wymarzony prezent
    Tragicz.dodajKupujacego(Jan); // są fundusze, ale Tradycjonalista nie kupuje w sieci
    Tragicz.dodajKupujacego(Piotr);
    Tragicz.dodajKupujacego(Wisia);
    Tragicz.dodajKupujacego(Pawel);
    Tragicz.dodajKupujacego(Bajtek);
    Tragicz.dodajKupujacego(Alicja);

    // ------------------ Symulacja -------------------
    
    cout << "Symulacja" << "\n";

    Tragicz.symuluj();
    return 0;
}