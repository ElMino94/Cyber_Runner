# Projet — Runner : Génération procédurale & Gameplay

## Création du projet (Termina Engine)

### Prérequis

Avant de lancer le projet, il est impératif d’avoir :

- le moteur **Termina Engine**
- le projet du jeu
- **xmake** installé sur la machine

---

### Génération de la solution

1. Ouvrir un terminal (cmd)

2. Se placer dans le dossier du projet :


cd Documents/MonProjet


(Adapter le chemin en fonction de l’emplacement du projet)

3. Générer la solution Visual Studio :


xmake project -k vsxmake


Cette commande permet de créer automatiquement la solution du jeu.

---

### Ouverture du projet

- Ouvrir la solution générée avec Visual Studio
- Compiler tout le moteur complet
- Lancer l’éditeur du moteur

---

### Chargement de la scène

Une fois dans le moteur :

1. Cliquer sur le bouton **World** en haut à gauche
2. Ouvrir la map :


Assets/Maps/map_menu

# Attention :  
Lors de la première ouverture, une seconde requête peut apparaître.  
Il faut **l’annuler**, sinon la map peut être supprimée.

---

### Lancement du jeu

Une fois la scène chargée :

- Cliquer sur le bouton **Play**

Le jeu se lance alors et la boucle de gameplay démarre.

---

### Résumé

- Générer la solution avec `xmake`
- Ouvrir le projet
- Charger la map `map_menu`
- Cliquer sur **Play**

---

## Conventions générales

- Monde 3D classique :
  - X vers la droite
  - Y vers le haut
  - Z vers l’avant
- Le joueur se déplace automatiquement vers l’avant (axe Z)
- Le gameplay est basé sur un système de lanes (gauche, centre, droite)
- Le moteur utilisé est un moteur custom basé sur Termina Engine
- Le code gameplay est séparé du moteur via un module dédié

---

## Partie 1 — Principe du gameplay

### Objectif

Le jeu est un runner dans lequel le joueur doit :

- éviter des obstacles
- collecter des pièces
- utiliser des bonus (ex : jump boost)

---

### Principe général

Le joueur avance automatiquement.

Il contrôle uniquement :

- le déplacement latéral
- le saut

Le monde est généré dynamiquement devant lui.

---

### Système de lanes

Le déplacement est basé sur trois positions :

- gauche : -1
- centre : 0
- droite : 1

La position du joueur est calculée comme :


position.x = startPosition.x + lane * laneOffset


---

### Interaction avec les objets

Types d’objets :

- obstacles → mort
- collectibles → score
- bonus → effet temporaire

---

## Partie 2 — Génération procédurale

### Objectif

Créer un niveau infini en générant dynamiquement des patterns.

---

### Principe

Le niveau est généré ligne par ligne.

Chaque ligne contient :

- obstacles
- espaces libres
- bonus

---

### Représentation d’un pattern


[0, 1, 0]


- 0 = vide  
- 1 = obstacle  
- 2 = barricade  
- 3 = pièce  
- 4 = bonus  

---

### Placement des objets


x = baseX + laneIndex * laneWidth
z = spawnZ


---

### Gestion mémoire

Les objets trop éloignés derrière le joueur sont supprimés.

---

## Partie 3 — Système de joueur

### Déplacement


position.z += speed * deltaTime

Q = se déplacer à gauche
D = se déplacer à droite

Space = sauter


---

### Changement de lane

Le joueur se déplace vers une position cible :


targetX = startPosition.x + lane * laneOffset


---

### Saut


velocity -= gravity * deltaTime
position.y += velocity * deltaTime


---

### Bonus — Jump Boost

Activation :


jumpForce = baseJumpForce * multiplier


Expiration :

- retour à la normale après un timer

---

### Collision

- collectible → score
- bonus → activation effet
- obstacle → game over

---

## Partie 4 — Architecture du projet

### Organisation

- `Runtime` → moteur
- `Editor` → éditeur
- `GameAssembly` → gameplay

---

### Composants principaux

- `RunnerPlayerComponent`
- `Procedural`
- `Collectibles`
- `JumpBoostPickup`

---

### Enregistrement des composants


REGISTER_COMPONENT(...)


Sans ça, le composant n’apparaît pas dans l’éditeur.

---

## Partie 5 — Build du projet

### Principe

Le projet est modulaire.

Le gameplay (`GameAssembly`) doit être recompilé séparément.

---

### Étapes

- ouvrir le projet
- compiler
- rebuild `GameAssembly` après modification

---

### Limitations

- pas de hot reload complet
- redémarrage parfois nécessaire

---

### Problèmes fréquents

- composant invisible → pas enregistré
- changement non appliqué → pas rebuild
- bug → mauvais setup scène

---

## Partie 6 — Interface utilisateur

### Objectif

Fournir une interface simple permettant :

- de lancer une partie
- de quitter le jeu
- de gérer la fin de partie

---

### Menu principal

Le jeu propose un menu avec deux boutons :

- **Play**
- **Quit**

Fonctionnement :

- **Play** lance la boucle de jeu
- **Quit** ferme l’application

---

### Lancement du jeu

Lorsque le joueur appuie sur **Play** :

- la partie démarre
- le joueur est contrôlable
- la génération procédurale commence

---

### Game Over

Lorsque le joueur entre en collision avec un obstacle :

- le joueur est marqué comme mort
- la boucle de jeu s’arrête

Un écran de **Game Over** est affiché.

---

### Retour au menu

Après la mort :

- le jeu revient à l’état initial
- l’éditeur / moteur permet de relancer une partie

Le joueur peut alors :

- relancer une partie
- quitter le jeu

---

### Principe général

L’interface reste volontairement simple afin de :

- se concentrer sur le gameplay
- faciliter les tests
- éviter une complexité inutile

---

### Possibilités d’amélioration

- bouton "Restart"
- animations UI

## Conclusion

Le projet repose sur :

- génération procédurale
- système de lanes
- bonus dynamiques
- architecture modulaire

L’objectif est d’obtenir un gameplay simple, fluide et extensible
