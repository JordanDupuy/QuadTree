# 🌳 Développement d’un QuadTree

Projet développé dans le cadre du cours de **Développement Efficace (2024-2025)** par **Benjamin ALBOUY-KISSI**.  
Ce projet implémente une structure de données **QuadTree**, utilisée pour organiser des objets dans un espace 2D et optimiser les recherches spatiales, comme les détections de collisions dans un moteur de jeu.

---

## 🎯 Objectif du projet

- Implémenter une structure **QuadTree** robuste et performante.
- Comparer ses performances à une structure linéaire (`std::vector`) : insertion, recherche de collision, usage mémoire.
- Fournir plusieurs **modes d’exécution** pour observer les comportements dans différents contextes.
- Proposer des **tests unitaires** via le framework **Catch2**.

---

## 🗂️ Structure du projet

- `TQuadTree.h`, `QuadTree.h` : fichiers source
- `tests.cpp` : logique des tests
- `Release/`, `ReleaseWithMemInfo/` : dossiers générés à la compilation

---

## 🛠️ Compilation et utilisation

### ⚙️ Mode 1 : Analyse des performances (vitesse)

1. Ouvrir l’invite de commande.
2. Naviguer dans le dossier `Release` généré par Visual Studio.
3. Lancer les commandes suivantes :

```bash
.\QuadTree.exe [generate]
.\QuadTree.exe [performance] -s
```

### ⚙️ Mode 2 : Analyse avec suivi mémoire

1. Ouvrir l’invite de commande.
2. Naviguer dans le dossier `ReleaseWithMemInfo` généré par Visual Studio.
3. Lancer les commandes suivantes :

```bash
.\QuadTree.exe [generate]
.\QuadTree.exe [performance] -s
```
