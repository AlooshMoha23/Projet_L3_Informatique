# PROJET : Interface générique pour le suivi de l’exécution  

Ici vous trouverez tous les exemples de test que nous avons développé, qui nous ont permis de tester est de faire évoluer notre interface graphique.

# Objectif du projet

L’objectif est de créer une interface qui illustre l’évolution d’une application distribuée.  
Cette interface doit non seulement permettre la visualisation des états des processus et les connexions entre eux, mais aussi fournir une API pour les développeurs d’applications distribuées. Cette API leur permettra d’envoyer les états de leur application à l’interface et de visualiser l’évolution de l’application en temps réel.

# Composition du github
Hitorique des progrès:
 - Etape 1 : Simple site qui envoie une information ( 1 ou 0 ) à l'interface graphique à intervalles réguliers.
 - Etape 2 : Deux sites qui envoient une information ( 1 ou 0 ) à l'interface graphique à intervalles réguliers.
 - Etape 3 : Nous avons développé deux sites ayant deux états (actif et en attente), l'état “attente” été envoyé lorsque le site était en attente de recevoir un message de l'autre site et l'état “actif” été envoyé lorsque l'on reçoit le message. Chacun envoie son état au serveur principal qui l’affiche graphiquement.
 - Etape 4 : Nous avons reproduit l'étapes précédente tout en ajoutant un graphe et des couleurs dans l'interface graphique.
Main:
 - Notre interface graphique et l'API.
Tests:
 - Un serveur orchestrateur et un site qui testent l'interface graphique pour un réseau en anneau et un réseau complet.

# Personnes impliquées

Membre du Groupe :
- Jalal Azouzout 
- Ali Ba Faqas
- Leila Bourouf
- Saghar Yazdani

Encadrante : 

Hinde Bouziane
