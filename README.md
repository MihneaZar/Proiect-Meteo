# Proiect-Meteo
Proiectul "Aparat meteo cu predicție de ploaie" din cadrul materiei Proiectarea cu Microprocesoare.

Biblioteci folosite: avr/io.h, avr/interrupt.h, stdio.h.  
Surse 3rd party: https://github.com/Matiasus/SSD1306/tree/master/lib, https://github.com/Sylaina/bme280.

Funcții: în src/main.c, init_all inițiază toate modulele necesare pentru rulare, iar Set_time este funcția inițială de setare a timpului.  
În lib, directoarele usart, spi, respectiv twi conțin funcțiile pentru comunicarea respectivă.
Folder-ul timer și button_interrupt sunt pentru numărarea milisecundelor și pentru butoane,
în timp ce sd_reader, ssd1306 și bme280 sunt pentru folosirea cititorului de microsd, a ecranului lcd și a senzorului.
Directorul miscellaneous conține fișierele pentru afișarea informațiilor pe lcd și preluarea datelor meteo de la senzor.

Demo aparat: https://youtu.be/tAVXPxxvXbI
