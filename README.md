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

## Utilisation
Le [Wiki](../../wiki) décrit davantage l'utilisation, mais le principe général est qu'il faut allumer un premier pod et attendre quelques secondes. Celui-ci devient le pod principal : on voit d'abord la lumière tourner sur le pod (recherche du wifi), puis faire une petit flash vert : ceci indique que le pod a créé un réseau wifi et attend des connexions. Les autres pods qui s'allumeront se relieront maintenant à celui-ci. Vous pouvez aussi vous connecter au réseau wifi créé. Il s'appelle PhysioPod0, et le mot de passe est "0123456789", en vous connectant, vous serez redirigé vers une page web, c'est là que vous pourrez interragir avec les pods, par exemple pour lancer un mode de jeu ou consulter les résultats.

## Créer ses propres pods
Le processus est décrit dans le [Wiki](../../wiki)


