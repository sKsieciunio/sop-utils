Mamy firmę, w której każdy pracownik uzywa swojej tablicy magnesowej donapisania jakiegos napisu motywujacego go do pracy.
Pewnego dnia przyszedl do tej firmy zlodziej magnesow literkowych i ukradl wszystkie znaki. 
Rozklad pokoi w firmie zasymulowany jest poprzez folder data1. W tym folderze kazdy folder to drzwi do kolejnego korytarza, 
a plik TXT to plik do drzwi, gdzie znajduje sie biedny orabowany pracownik.

W Etapie 1
Bedziemy chodzic po kazdych pokojach i witac sie z kazdym pracownikiem. Kazdy pracownik w firmie
to bedzie osobny watek, ktory bedzie sie z nami wital.
W tym etapie w naszym programie dodajemy możliwość przetwarzania plików w katalogu.
Program nie wchodzi jeszcze do środka plików, ale przechodzi po strukturze katalogów, 
sprawdzając, które pliki znajdują się w podanych folderach. Dla każdego pliku tworzony 
jest osobny wątek, który później zajmie się zliczaniem znaków w tym pliku. Warto zauważyć, 
że na tym etapie program jedynie "rozpoznaje" strukturę katalogów, nie wchodząc jeszcze w głąb 
żadnego z plików.

W Etapie 2
Kazdy z pracownikow wchodzi do swojego pokoju i liczy ile brakuje mu jego magnesow kazdego typu.
Informuje nas o tym, aby firma w której pracuje mogła zakupić odpowiednią ich ilość, 
aby pracownicy mogli odzyskac motywacje do pracy dzieki swoim napisom. 
W tym etapie przechodzimy do głównego zadania – zliczania wystąpień znaków w plikach. Teraz 
wątki, które zostały utworzone w poprzednim etapie, zaczynają rzeczywiście przetwarzać pliki, 
wchodząc do środka i analizując zawartość. Każdy wątek otwiera plik, przegląda go znak po znaku 
i zlicza, jak często dany znak występuje. Aby operacja ta była bezpieczna w kontekście 
wielowątkowości, używamy mutexów, które zapewniają, że operacje na zmiennej globalnej (licznikach) 
nie będą się nakładały. W tym etapie program już rzeczywiście wykonuje główne zadanie – zliczanie
znaków.

W Etapie 3
Kierownicy są zaniepokojeni postępem zliczania liter przez swoich pracowników. Dlatego zależy im 
na regularnym monitorowaniu wyników. Co 0,1 sekundy chcieliby otrzymywać informacje o bieżącym 
stanie zliczania liter.
Napisz obsługę sygnału SIGUSR1, która po jego odebraniu wyświetli aktualny stan zliczania liter.
Nastepnie wysyłaj sygnał SIGUSR1 co 0.1 sekundy aby moc obserowawc progres pracy.

W Etapie 4
Może się jednak okazać, że złodziej zostanie znaleziony. W takim przypadku należy poinformowac
wszytskich pracowników aby nie kontunuowali zliczania brakujących im magnesów. Informację o 
znalezieniu złodzieja będzie sygnalizować SIGINT
W tym etapie program zyskuje funkcjonalność, która umożliwia jego zatrzymanie w dowolnym 
momencie za pomocą sygnału SIGINT (np. naciśnięcie Ctrl+C w terminalu). Dzięki dodaniu obsługi 
sygnałów i użyciu odpowiednich mechanizmów wątków (np. pthread_sigmask i sigwait), program w 
każdej chwili może zakończyć swoje działanie, nawet jeśli w danym momencie przetwarza pliki. 
Dzięki temu użytkownik ma pełną kontrolę nad procesem, może wstrzymać pracę programu w dowolnym 
momencie, jeśli zajdzie taka potrzeba.