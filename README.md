# Physiopods

## Qu'est-ce que c'est ?
Le but de ce projet est de créer des objets (des pods), utilisés pour jouer, pour la performance sportive ou pour la santé. Différents modes de jeu sont possibles, mais il est par exemple proposé un jeu de réflexes ou il faut appuyer le plus vite possible sur un pod lorsqu'il s'allume. Le score est alors basé sur le délai qu'a mis le joueur, mais aussi sur d'éventuelles erreurs (s'il a appuyé sur un pod éteint).

### Produits similaires
L'idée n'est pas nouvelle. De nombreux produits similaires existent [commercialement](https://www.joueclub.fr/jeux-de-societes/quick-touch-8421134091719.html), ou en projets [open-source](https://github.com/projectswithalex/Reaction-Lights-Training-Module/tree/Version-2).
Nous avons souhaité proposer une alternative avec des objectifs qui nous semblent différents.

### Objectifs du projet
- Nombre de pods : il doit être possible d'utiliser **beaucoup** de pods (la limite actuelle est de 255). Tous les pods sont interchangeables, il n'y a pas de maître ou d'esclave.
- Bon marché : le coût de revient ne doit pas être trop important. Il doit aussi être possible de réparer, de changer des composants ou la batterie.
- Interface web : pour éviter de multiplier les boutons, le gros de l'interface est proposée sur un site web hébergé par les pods. On peut y choisir un mode de jeu, consulter des résultats ou régler des paramètres.
- Polyvalent : les pods doivent pouvoir être posés au sol, fixés avec des sangles ou des ventouses pour permettre des usages différents. De même, le code est prévu pour qu'il soit assez facile d'ajouter d'autres contrôleurs (passer d'un bouton à un capteur capacitif, à un capteur de proximité ou encore à un capteur piezo)

## Comment créer mes propres pods
On y travaille...

Vous pouvez utiliser la [Feuille de route des différents composants](https://lite.framacalc.org/qh2ci1g483-a8e7) pour visualiser les différents composants nécessaires à la fabrication de vos Pods. Entrez la quantité de Pod désirée et vous obtiendrez un aperçu du coùt de fabrication ainsi que les quantités à acheter.

Pour acheter les différents composants, voici une [Liste d'achat AliExpress](https://www.aliexpress.com/p/wishlist/shareReflux.html?groupId=afP5eqgDhE6RQVqTO1i1rTBBvAYjB%2Fnj%2Bz6Nbt9ddoo%3D). Pensez à bien vérifier les caractéristiques des différents produits (valeur de résistance, nombre de led sur le ring, etc) selon les valeurs indiquées sur la feuille de route vue précédemment.

### Impression du boitier

### Instructions de montage du boitier Pod

**NE PAS PREPARER LES ESP AVEC LEURS HEADERS AVANT QUE CE NE SOIT DEMANDE !!**

**Préparation de l'anneau de Leds :**
- Préparer 3 fils de 10 cm : un noir pour la masse, un rouge pour le positif et un coloré pour la data DI _(vert dans notre exemple)_ ;
- Dénuder les extrémités et les étamer ;
- Préparer les bornes GND, DI et 5V avec du flux et étamer ;
- Souder les fils avec les couleurs correspondantes : noir -> GND, rouge -> 5V et vert -> DI ;
- Ajouter de la colle chaude sur les soudures en maintenant les fils plaqués pour les sécuriser.

**Souder les composants au PCB :** _Pour se repérer dans les emplacements des composants, cf schéma et s'aider des marques sur le PCB._
1. Les headers du module de charge :
- Positionner 4 headers sur les emplacements prévus du module de charge Bat+, Bat-, In+ et In- avec **le bout court dans le PCB** ;
- Placer le module de charge sur les headers : il ne sera pas soudé tout de suite mais il servira à bien orienter les headers ;
- Maintenir le module de charge au PCB avec une petite pince ou de la patafix ;
- Souder les headers au PCB ;
- Oter le module de charge.
2. Les headers de l'ESP :
- Positionner les deux barrettes de headers sur les emplacements prévus pour l'ESP avec **le bout court dans le PCB** ;
- Placer l'ESP sur les headers : il ne sera pas soudé tout de suite mais il servira à bien orienter les headers ;
- Maintenir l'ESP au PCB avec une petite pince ou de la patafix ;
- Souder les headers au PCB ;
- Oter l'ESP.
3. La résistance de 1M ohm :
- Courber les deux pattes de la résistance pour former un U ;
- Insérer la résistance dans son emplacement ;
- Souder les pattes de la résistance ;
- Couper avec une pince l'excédent
4. L'anneau de Leds :
- A l'emplacement prévu, souder le fil noir sur GND, le fil rouge sur 3,3v et le dernier fil coloré sur le pin 8 de l'ESP.
5. Le capteur Piezo :
- A l'emplacement prévu, souder le fil noir sur le GND restant et le fil rouge sur le pin 5 de l'ESP.
6. Le connecteur de batterie :
- A l'emplacement prévu, souder le connecteur coudé orienté vers l'extérieur du PCB.
7. L'ESP :
- Placer et souder l'ESP sur les headers précédement installés.
8. Le module de charge :
- Placer et souder le module de charge sur les headers précédement installés ;
- Couper avec une petite pince les extrémités des headers dépassant de la soudure.
9. L'interrupteur :
- A l'emplacement prévu, souder l'interrupteur.



## Comment ça marche ?
Les Physiopods reposent sur des ESP32. De nombreuses options doivent être possibles, mais les tests ont surtout été faits sur des [ESP32-S3](https://www.espressif.com/en/products/socs/esp32-s3). Ces composants sont capables de créer un réseau wifi, ce qui permet à tous les pods de se connecter entre eux, mais aussi à l'utilisateur de se connecter. Il est alors redirigé vers l'interface utilisateur, qui va lui permettre d'interragir avec les pods (par exemple pour lancer une partie, ou pour consulter les scores).

### Architecture générale
Tous les pods sont interchangeables, mais le premier pod allumé va prendre un rôle différent et sera désigné comme ServerPod : c'est lui qui va créer le réseau wifi et c'est lui qui centralisera tout. Les autres pods seront des ClientPod et ne feront que recevoir des ordres du ServerPod (par exemple pour qu'il s'allume en vert clignotant), ou pour envoyer des informations au ServerPod (par exemple pour dire qu'il a été activé par le joueur).
![description générale](docs/Overview.png)
