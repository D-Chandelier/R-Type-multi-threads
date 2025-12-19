Le fichier main.cpp est le fichier principal du jeu. Il contient la fonction main() qui crée une instance de la classe Game et appelle sa méthode run() pour démarrer le jeu. Il déclenche "enet_initialize" à l'ouverture et "enet_deinitialize" pour etre sur de ne pas avoir de problème de fermeture prématurée sur la logique Enet
Le main.cpp declenche Game::run() qui fait tourner le jeu tant que la fenêtre est ouverte.

Game::run() est la boucle principale du jeu. Elle gère les événements de la fenêtre, met à jour les objets du jeu et dessine l'écran:
- Le menu main.
- Le menu join
- Le menu option
Ces trois menu sont gérer par la boucle Game::run().
Suivant le menu actif, la boucle Game::run() appelle la méthode correspondante pour gérer les événements et mettre à jour l'écran:
- Main menu permet la sélection de PLAY, JOIN SERVER, OPTION ou EXIT(Quitter le jeu).
- Join server permet de rejoindre un serveur en entrant son adresse IP et son port. Un affichage des serveurs visibles sur le reseau permet une selection rapide. (Sous Windows, l'annonce broadcast des serveurs peut etre mal interprétée et ne pas apparaitre. Pour rejoindre une partie, il suffit de renseigner l'addresse IP d'un serveur et de son port pour le rejoindre)
- Option permet de modifier les options. (PORT utiliser en mode serveur. Le serveur est par défaut sur le port 1234. Le client peut modifier le port du serveur pour paramétrer un autre port. Suivant le port choisi, le serveur utilisera le port suivant pour la fonction broadcast d'annonce sur le reseau. Ex: port de jeu 1234, port d'annonce 1235). 
Bien sûr, les 2  ports utilisés doivent être ouvert sur les parefeus des machines (voir redirigés par un routeur pour des acces distants).

La logique du programme va tournée sur 3 éléments distincts:

- La partie Game qui va gérer toute la partier locale du joueur (Boucle sur le jeu, gestion des menus, update et draw des éléments du jeu, quelque déclenchement d'envoi au serveur comme le tire.).
le partie Game ne tourne pas sur thread et depend donc des caractéristiques de la machine.
Elle va aussi démarrer le client et le serveur suivant le choix de jeu (PLAY-> serveur lancé en local puis lancement/connection du CLIENT au serveur. JOIN SERVER-> client lancé en local puis connection au serveur distant).

- La partie Client qui va gérer la communication avec le serveur (Envoi et réception des messages).
Le client tourne sur thread et est donc indépendant du thread principal du jeu.
Suivant le FPS renseigné dans Config.hpp(Config::Get().frameRate), le client va envoyer/recevoir des messages, mettre à jour les informations de jeu comme la position des entitées pour que la pertie Game puisse les gérer.

- La partie Serveur qui va gérer la communication avec les clients (Envoi et réception des messages). Sa boucle est aussi cadencée par le FPS

Le programme est donc composé de 3 éléments distincts, mais qui interagissent entre eux pour fonctionner.