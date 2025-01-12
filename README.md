
# Používateľská príručka
## CLI Blackjack

Pre spustenie aplikácie mimo servera frios2 je nutné mať nainštalovaný cmake a gcc.
pre začiatok odporúčam updatovať package\
`sudo apt-get update`
\
gcc - `sudo apt-get install build-essential gdb`\
cmake - `sudo apt-get install cmake`

Následne naklonovať repozitár pomocou použitia `git clone https://github.com/kubeez1337/CLIBlackjack.git`
V naklonovanom repe vytvoriť zložku build odkiaľ budem spúšťať vybuildené súbory.\
1.`mkdir build`\
2.`cd build`\
3.`cmake ..`\
4.`make`\
5.`./main`\
Po spustení aplikácie zadá úžívateľ svoje meno pod ktorým bude hrať.
Najprv vytvorte server pomocou  `s` ako start. Zadajte meno a port servera.
Po spustení servera ste vyzvaní, aby ste zadali stávku na toto kolo.\
Hráč začína s bilanciou 100 chipov.
Po zadaní sú vám rozdané karty a vy máte možnosť voľby, či chcete ďalšiu kartu, alebo nie, keď sa rozhodnete potiahnuť pomocou napísania `hit`
je Vám potiahnutá karta a jej číselná hodnota je pričítaná do sumy kariet. Ak presiahnete hodnotu 21, stav vašej ruky je "bust" a teda za každých okolností po dokončení dealerovho ťahu, stávku prehráte. Ak sa po sekvencií ťahov kariet vhodne priblížite k 21, povedzme napríklad počtom 20, alebo 19, je ideálne stáť a prenechať ťah kariet na dealera.
Dealer ťahá, kým nemá sumu hodnôt kariet >= 17. Ak dealerová_suma_kariet < hráčova_suma_kariet, hráč vyhráva dvojnásobok počiatočnej stávky, v opačnom prípade hráč prehráva.
Po skončení kola hráč môže hrať ďalej, so zvyšnými kartami, alebo sa rozhodnúť stôl opustiť. Aktivita servera sa končí po vytiahnutí všetkých 52 kariet z hracieho balíčku.
Následná ukážka jednoho herného cyklu:
```
roman@MYSUPERPC:~/blackjack/build$ ./main
Enter your name: Roman
Hello Roman! Do you want to start a server or join an existing one?
 Type e to exit
(s/j/e): s
Enter server name: BlackjackServer
Enter server port: 8082
Server started with PID 95685
Server 'BlackjackServer' is running on port 8082
Connected to server 'BlackjackServer'
Your balance: 100 
If you want to exit type 'exit'
Enter amount that you would like to bet: 
10
Dealer: 2|D, ?
Sum: 2

Roman: 5|S, Q|H, 
Roman Sum: 15
--------------------------------
Enter 'hit' to draw a card, 'stand' to end your turn: 
hit  
Roman: 5|S, Q|H, 10|H, 
Roman Sum: 25
--------------------------------

Dealer hits
--------------------------------
Dealer: 2|D, 8|C, K|C, 

Dealer Sum: 20
--------------------------------
Roman: 5|S, Q|H, 10|H, 
Roman Sum: 25
--------------------------------
--------------------------------

You bust! Dealer wins.

Your balance: 90 
If you want to exit type 'exit'
Enter amount that you would like to bet: 
```


