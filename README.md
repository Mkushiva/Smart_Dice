To solidna baza! Zawiera najważniejsze informacje, ale jako projekt inżynierski warto ubrać to w nieco bardziej profesjonalną strukturę Markdown. Dzięki temu recenzent od razu zobaczy, że podchodzisz do tematu systemowo.

Oto propozycja, jak możesz to sformatować, aby wyglądało przejrzyście na GitHubie:

Sugerowana treść pliku README.md (skopiuj i wklej):
Markdown
# Interactive Die - Firmware

Repozytorium zawiera kod źródłowy oprogramowania dla fizycznej części projektu inżynierskiego: **"Inteligentna kostka losująca z wyświetlaczem, ładowaniem bezprzewodowym i komunikacją z aplikacją mobilną"**.

## Opis projektu
Oprogramowanie obsługuje mikrokontroler [WPISZ MODEL, np. Seeed Studio XIAO nRF52840], zarządzając:
* Odczytem danych z akcelerometru w celu detekcji rzutu.
* Generowaniem wyników losowych.
* Wyświetlaniem grafiki na ekranie [WPISZ TYP, np. OLED].
* Komunikacją Bluetooth Low Energy (BLE) z dedykowaną aplikacją mobilną.

## Autorzy
* **Autor:** Kamil Adamek
* **Promotor:** Dr inż. Krzysztof Hanzel

## Wymagane biblioteki
Do poprawnej kompilacji projektu w środowisku Arduino IDE wymagane są następujące biblioteki (dostępne w Library Manager):
* **Adafruit_nRF52-Arduino** – obsługa rdzenia procesora i stosu BLE.
* **Seeed_Arduino_LSM6DS3** – obsługa 6-osiowego akcelerometru/żyroskopu.
* **U8g2** – biblioteka graficzna do obsługi wyświetlacza.

## Kluczowe funkcjonalności
* **BLE Service:** Rozgłaszanie (Advertising) wyniku rzutu oraz stanu baterii.
* **Energy Management:** Implementacja trybów uśpienia mikrokontrolera pomiędzy rzutami.
* **IMU Processing:** Wykrywanie stabilizacji kostki po rzucie (detekcja spoczynku).

## Instalacja
1. Dodaj odpowiednią płytkę do menedżera płytek (np. Seeed nRF52 Boards).
2. Zainstaluj powyższe biblioteki.
3. Skonfiguruj parametry BLE (UUID) zgodnie z kodem aplikacji mobilnej.
4. Skompiluj i wgraj program na urządzenie.
