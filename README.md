# Début de tests autour d'un projet de physiopods
Pour l'instant l'objectif est juste d'avoir un proof of concept, le physiopod doit être capable d'être un serveur wifi et http.

## Quel ESP utiliser ?
Pour l'instant ce n'est testé que sur ESP32-S3
l'ESP01 ne semble pas assez puissant (tous les ESP8266 doivent donc être concernés ?).
NB : En mettant isDebug à false dans isDebug.h, on doit pouvoir améliorer un peu les performances : la plupart des écritures sur le port séries seront desactivées.

## Montage
Il faudrait écrire ici un montage "de base"
les pins utilisées pour les boutons et LED sont écrits dans pins.h

## Créer un nouveau contrôleur pour le PhysioPode
Il faudrait expliquer comment créer un nouveau Controleur.
Un Controleur est la façon dont l'utilisateur interragit avec les PhysioPods. Par exemple appuyer sur le bouton, ou passer devant un détecteur IR, ou encore...
On peut créer un nouveau Controleur en héritant de PysioPodControl. ButtonControl est prévu comme un exemple de contrôleur.

## Créer un nouveau PhysioMode
On doit pouvoir créer de nouveaux modes. Un mode est une classe héritant de PhysioPodMode. FastPressMode est prévu comme une exemple d'un mode basique