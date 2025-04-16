//Vous n'avez absolument pas le droit de modifier ce fichier !

#include <iostream>
#include <fstream>
#include <algorithm>
#include <random>
#include <set>
#include <crtdbg.h>

#include "catch_amalgamated.hpp"
#include "QuadTree.h"

const SLimits subDataLimits = { 0.42f, 0.43f, 0.72f, 0.73f };
#ifdef _DEBUG
const size_t datasetSize = 1000000;
const char* datasetFilename = "dataset_debug.dat";
#else
const size_t datasetSize = 10000000;
const char* datasetFilename = "dataset.dat";
#endif

/**
 * @brief Lit un fichier de données de test.
 * 
 * Cette fonction lit un fichier de données de test contenant des rectangles de taille et de position aléatoires.
 * Pour chaque rectangle, la fonction de rappel est appelée avec les coordonnées du coin supérieur gauche et inférieur droit.
 * 
 * Elle lit également la profondeur maximale et la taille du QuadTree théorique correspondant à ces données.
 * 
 * @param [out] depth Profondeur maximale du QuadTree théorique
 * @param [out] datasetSize Nombre de rectangles dans le fichier
 * @param callback Fonction de rappel appelée pour chaque rectangle
 */
void readDataSet(size_t& depth, size_t& datasetSize, std::function<void(float x1, float y1, float x2, float y2)> callback) {
  std::ifstream file(datasetFilename, std::ios_base::binary);
  REQUIRE(file.is_open());
  file.read(reinterpret_cast<char*>(&datasetSize), sizeof(datasetSize));
  for (size_t i = 0; i < datasetSize; i++)
  {
    struct {
      float x1, y1, x2, y2;
    } r;
    file.read(reinterpret_cast<char*>(&r), sizeof(r));
    if (file.gcount() == sizeof(r))
      callback(r.x1, r.y1, r.x2, r.y2);
  }
  file.read(reinterpret_cast<char*>(&depth), sizeof(depth));
}

/**
 * @brief Génère un fichier de données pour les tests.
 * 
 * Ce test génère un fichier de données contenant des rectangles de taille et de position aléatoires.
 * Il est utilisé pour les tests de stress et de performance.
 * 
 * @note Ce test est caché et doit être exécuté explicitement par la ligne de commande
 */
TEST_CASE("TQuadTree.0-Generating test fixtures", "[.generate]") {
  std::random_device rd;
  std::default_random_engine dre(rd());
  std::uniform_real_distribution<float> urd(0.0f, 1.0f);
  std::ofstream file(datasetFilename, std::ios_base::binary);

  file.write(reinterpret_cast<const char*>(&datasetSize), sizeof(datasetSize));
  size_t depth = 0;
  for (size_t i = 0; i < datasetSize; i++)
  {
    struct {
      float x1, y1, x2, y2;
    } r, limits;
    float width = urd(dre) * 0.1f;
    float height = urd(dre) * 0.1f;
    r.x1 = urd(dre) * (1.0f - width);
    r.y1 = urd(dre) * (1.0f - height);
    r.x2 = r.x1 + width;
    r.y2 = r.y1 + height;
    file.write(reinterpret_cast<const char*>(&r), sizeof(r));
    size_t currentDepth = 0;
    limits = { 0.0f, 0.0f, 1.0f, 1.0f };
    while (r.x1 >= limits.x1 && r.x2 <= limits.x2 && r.y1 >= limits.y1 && r.y2 <= limits.y2)
    {
      currentDepth++;
      float midX = (limits.x1 + limits.x2) / 2.0f;
      float midY = (limits.y1 + limits.y2) / 2.0f;
      if (r.x1 < midX && r.y1 < midY)
      {
        limits.x2 = midX;
        limits.y2 = midY;
      }
      else if (r.x1 >= midX && r.y1 < midY)
      {
        limits.x1 = midX;
        limits.y2 = midY;
      }
      else if (r.x1 < midX && r.y1 >= midY)
      {
        limits.x2 = midX;
        limits.y1 = midY;
      }
      else
      {
        limits.x1 = midX;
        limits.y1 = midY;
      }
    }
    if (currentDepth > depth)
      depth = currentDepth;
  }
  file.write(reinterpret_cast<const char*>(&depth), sizeof(depth));
}

/**
 * @brief Teste les fonctions de base du QuadTree.
 *
 * Ce test vérifie le bon fonctionnement des fonctions de base du QuadTree.
 */
TEST_CASE("TQuadTree.1-QuadTree basic test", "[basic]") {
  //Crée un QuadTree vide
  QuadTree qt;
  REQUIRE(qt.empty() == true);
  REQUIRE(qt.size() == 0);
  REQUIRE(qt.limits() == SLimits{ 0.0f, 0.0f, 1.0f, 1.0f });
  REQUIRE(qt.depth() == 1);

  //Insère un rectangle de taille 1x1 (pleine taille)
  qt.insert(Rectangle(0.0f, 0.0f, 1.0f, 1.0f));
  REQUIRE(qt.empty() == false);
  REQUIRE(qt.size() == 1);
  REQUIRE(qt.limits() == SLimits{ 0.0f, 0.0f, 1.0f, 1.0f });
  REQUIRE(qt.depth() == 1);

  //Tente d'insérer des rectangles invalides
  REQUIRE_THROWS_AS(qt.insert(Rectangle(-0.5f, 0.0f, 1.0f, 1.0f)), std::domain_error);
  REQUIRE_THROWS_AS(qt.insert(Rectangle(0.0f, -0.5f, 1.0f, 1.0f)), std::domain_error);
  REQUIRE_THROWS_AS(qt.insert(Rectangle(0.0f, 0.0f, 1.1f, 1.0f)), std::domain_error);
  REQUIRE_THROWS_AS(qt.insert(Rectangle(0.0f, 0.0f, 1.0f, 1.1f)), std::domain_error);

  //Réinitialise le QuadTree
  qt.clear();
  REQUIRE(qt.empty() == true);
  REQUIRE(qt.size() == 0);
  REQUIRE(qt.limits() == SLimits{ 0.0f, 0.0f, 1.0f, 1.0f });
  REQUIRE(qt.depth() == 1);

  //Insère un rectangle de pleine taille
  qt.insert(Rectangle(0.0f, 0.0f, 1.0f, 1.0f));
  //Insère 4 rectangles de taille 0.5x0.5 dans chaque coin de la surface
  qt.insert(Rectangle(0.0f, 0.0f, 0.5f, 0.5f)); //NO
  qt.insert(Rectangle(0.5f, 0.0f, 1.0f, 0.5f)); //NE
  qt.insert(Rectangle(0.0f, 0.5f, 0.5f, 1.0f)); //SO
  qt.insert(Rectangle(0.5f, 0.5f, 1.0f, 1.0f)); //SE
  REQUIRE(qt.empty() == false);
  REQUIRE(qt.size() == 5);
  REQUIRE(qt.limits() == SLimits{ 0.0f, 0.0f, 1.0f, 1.0f });
  REQUIRE(qt.depth() == 2);

  SECTION("container") {
    //Vérifie que tous les rectangles sont bien présents
    auto all = qt.getAll();
    REQUIRE(all.size() == 5);
    REQUIRE(std::count(all.begin(), all.end(), Rectangle(0.0f, 0.0f, 1.0f, 1.0f)) == 1); //Surface totale
    REQUIRE(std::count(all.begin(), all.end(), Rectangle(0.0f, 0.0f, 0.5f, 0.5f)) == 1); //NO
    REQUIRE(std::count(all.begin(), all.end(), Rectangle(0.5f, 0.0f, 1.0f, 0.5f)) == 1); //NE
    REQUIRE(std::count(all.begin(), all.end(), Rectangle(0.0f, 0.5f, 0.5f, 1.0f)) == 1); //SO
    REQUIRE(std::count(all.begin(), all.end(), Rectangle(0.5f, 0.5f, 1.0f, 1.0f)) == 1); //SE

    //Vérifie que les quadrants sont bien remplis avec leurs uniques rectangles respectifs
    //NO
    auto quadrant = qt.findInscribed({ 0.0f, 0.0f, 0.5f, 0.5f });
    REQUIRE(quadrant.size() == 1);
    REQUIRE(std::count(quadrant.begin(), quadrant.end(), Rectangle(0.0f, 0.0f, 0.5f, 0.5f)) == 1);
    //NE
    quadrant = qt.findInscribed({ 0.5f, 0.0f, 1.0f, 0.5f });
    REQUIRE(quadrant.size() == 1);
    REQUIRE(std::count(quadrant.begin(), quadrant.end(), Rectangle(0.5f, 0.0f, 1.0f, 0.5f)) == 1);
    //SO
    quadrant = qt.findInscribed({ 0.0f, 0.5f, 0.5f, 1.0f });
    REQUIRE(quadrant.size() == 1);
    REQUIRE(std::count(quadrant.begin(), quadrant.end(), Rectangle(0.0f, 0.5f, 0.5f, 1.0f)) == 1);
    //SE
    quadrant = qt.findInscribed({ 0.5f, 0.5f, 1.0f, 1.0f });
    REQUIRE(quadrant.size() == 1);
    REQUIRE(std::count(quadrant.begin(), quadrant.end(), Rectangle(0.5f, 0.5f, 1.0f, 1.0f)) == 1);

    //Teste la fonction findColliding
    auto rects = qt.findColliding({ 0.75f, 0.25f, 0.9f, 0.75f });
    REQUIRE(rects.size() == 3); //Surface totale, NE, SE
    REQUIRE(std::count(rects.begin(), rects.end(), Rectangle(0.0f, 0.0f, 1.0f, 1.0f)) == 1); //Surface totale
    REQUIRE(std::count(rects.begin(), rects.end(), Rectangle(0.5f, 0.0f, 1.0f, 0.5f)) == 1); //NE
    REQUIRE(std::count(rects.begin(), rects.end(), Rectangle(0.5f, 0.5f, 1.0f, 1.0f)) == 1); //SE

    //Supprime le rectangle correspondant au quadrant SO
    qt.remove(Rectangle(0.0f, 0.5f, 0.5f, 1.0f));
    //Vérifie que tous les rectangles sont bien présents
    all = qt.getAll();
    REQUIRE(all.size() == 4);
    REQUIRE(std::count(all.begin(), all.end(), Rectangle(0.0f, 0.0f, 1.0f, 1.0f)) == 1); //Surface totale
    REQUIRE(std::count(all.begin(), all.end(), Rectangle(0.0f, 0.0f, 0.5f, 0.5f)) == 1); //NO
    REQUIRE(std::count(all.begin(), all.end(), Rectangle(0.5f, 0.0f, 1.0f, 0.5f)) == 1); //NE
    REQUIRE(std::count(all.begin(), all.end(), Rectangle(0.5f, 0.5f, 1.0f, 1.0f)) == 1); //SE

    //Supprime le rectangle de pleine taille
    qt.remove(Rectangle(0.0f, 0.0f, 1.0f, 1.0f));
    //Vérifie que tous les rectangles sont bien présents
    all = qt.getAll();
    REQUIRE(all.size() == 3);
    REQUIRE(std::count(all.begin(), all.end(), Rectangle(0.0f, 0.0f, 0.5f, 0.5f)) == 1); //NO
    REQUIRE(std::count(all.begin(), all.end(), Rectangle(0.5f, 0.0f, 1.0f, 0.5f)) == 1); //NE
    REQUIRE(std::count(all.begin(), all.end(), Rectangle(0.5f, 0.5f, 1.0f, 1.0f)) == 1); //SE
  }

  SECTION("iterator") {
    //Vérifie que tous les rectangles sont bien présents
    REQUIRE(std::distance(qt.begin(), qt.end()) == 5);
    REQUIRE(std::count(qt.begin(), qt.end(), Rectangle(0.0f, 0.0f, 1.0f, 1.0f)) == 1); //Surface totale
    REQUIRE(std::count(qt.begin(), qt.end(), Rectangle(0.0f, 0.0f, 0.5f, 0.5f)) == 1); //NO
    REQUIRE(std::count(qt.begin(), qt.end(), Rectangle(0.5f, 0.0f, 1.0f, 0.5f)) == 1); //NE
    REQUIRE(std::count(qt.begin(), qt.end(), Rectangle(0.0f, 0.5f, 0.5f, 1.0f)) == 1); //SO
    REQUIRE(std::count(qt.begin(), qt.end(), Rectangle(0.5f, 0.5f, 1.0f, 1.0f)) == 1); //SE

    //Vérifie que les quadrants sont bien remplis avec leurs uniques rectangles respectifs
    //NO
    REQUIRE(std::distance(qt.beginInscribed({ 0.0f, 0.0f, 0.5f, 0.5f }), qt.end()) == 1);
    REQUIRE(std::count(qt.beginInscribed({ 0.0f, 0.0f, 0.5f, 0.5f }), qt.end(), Rectangle(0.0f, 0.0f, 0.5f, 0.5f)) == 1);
    //NE
    REQUIRE(std::distance(qt.beginInscribed({ 0.5f, 0.0f, 1.0f, 0.5f }), qt.end()) == 1);
    REQUIRE(std::count(qt.beginInscribed({ 0.5f, 0.0f, 1.0f, 0.5f }), qt.end(), Rectangle(0.5f, 0.0f, 1.0f, 0.5f)) == 1);
    //SO
    REQUIRE(std::distance(qt.beginInscribed({ 0.0f, 0.5f, 0.5f, 1.0f }), qt.end()) == 1);
    REQUIRE(std::count(qt.beginInscribed({ 0.0f, 0.5f, 0.5f, 1.0f }), qt.end(), Rectangle(0.0f, 0.5f, 0.5f, 1.0f)) == 1);
    //SE
    REQUIRE(std::distance(qt.beginInscribed({ 0.5f,0.5f,1.0f,1.0f }), qt.end()) == 1);
    REQUIRE(std::count(qt.beginInscribed({ 0.5f, 0.5f, 1.0f, 1.0f }), qt.end(), Rectangle(0.5f, 0.5f, 1.0f, 1.0f)) == 1);

    //Teste la fonction beginColliding
    REQUIRE(std::distance(qt.beginColliding({ 0.75f, 0.25f, 0.9f, 0.75f }), qt.end()) == 3); //Surface totale, NE, SE
    REQUIRE(std::count(qt.beginColliding({ 0.75f, 0.25f, 0.9f, 0.75f }), qt.end(), Rectangle(0.0f, 0.0f, 1.0f, 1.0f)) == 1); //Surface totale
    REQUIRE(std::count(qt.beginColliding({ 0.75f, 0.25f, 0.9f, 0.75f }), qt.end(), Rectangle(0.5f, 0.0f, 1.0f, 0.5f)) == 1); //NE
    REQUIRE(std::count(qt.beginColliding({ 0.75f, 0.25f, 0.9f, 0.75f }), qt.end(), Rectangle(0.5f, 0.5f, 1.0f, 1.0f)) == 1); //SE

    //Supprime le rectangle correspondant au quadrant SO
    qt.remove(Rectangle(0.0f, 0.5f, 0.5f, 1.0f));

    //Vérifie que tous les rectangles sont bien présents
    REQUIRE(std::distance(qt.begin(), qt.end()) == 4);
    REQUIRE(std::count(qt.begin(), qt.end(), Rectangle(0.0f, 0.0f, 1.0f, 1.0f)) == 1); //Surface totale
    REQUIRE(std::count(qt.begin(), qt.end(), Rectangle(0.0f, 0.0f, 0.5f, 0.5f)) == 1); //NO
    REQUIRE(std::count(qt.begin(), qt.end(), Rectangle(0.5f, 0.0f, 1.0f, 0.5f)) == 1); //NE
    REQUIRE(std::count(qt.begin(), qt.end(), Rectangle(0.5f, 0.5f, 1.0f, 1.0f)) == 1); //SE

    //Supprime le rectangle de pleine taille
    qt.remove(Rectangle(0.0f, 0.0f, 1.0f, 1.0f));
    //Vérifie que tous les rectangles sont bien présents
    REQUIRE(std::distance(qt.begin(), qt.end()) == 3);
    REQUIRE(std::count(qt.begin(), qt.end(), Rectangle(0.0f, 0.0f, 0.5f, 0.5f)) == 1); //NO
    REQUIRE(std::count(qt.begin(), qt.end(), Rectangle(0.5f, 0.0f, 1.0f, 0.5f)) == 1); //NE
    REQUIRE(std::count(qt.begin(), qt.end(), Rectangle(0.5f, 0.5f, 1.0f, 1.0f)) == 1); //SE
  }

  //Réinitialise le QuadTree
  qt.clear();
  REQUIRE(qt.empty() == true);
  REQUIRE(qt.size() == 0);
  REQUIRE(qt.limits() == SLimits{ 0.0f, 0.0f, 1.0f, 1.0f });
  REQUIRE(qt.depth() == 1);
}

TEST_CASE("TQuadTree.2-QuadTree advanced tests", "[advanced]")
{
  //Lit un fichier de données de test et initialise un QuadTree avec ces données.
  //Stocke égalements les rectangles dans deux ensembles pour vérifier leur présence dans le QuadTree
  //L'ensemble rectsAll contient tous les rectangles, rectsSub contient ceux qui sont inclus dans subDataLimits
  QuadTree qt;
  size_t depth;
  size_t datasetSize;
  std::multiset<Rectangle> rectsAll;
  std::multiset<Rectangle> rectsSub;
  std::multiset<Rectangle> rectsSubCollide;
  readDataSet(depth, datasetSize, [&qt, &rectsAll, &rectsSub, &rectsSubCollide](float x1, float y1, float x2, float y2) {
    qt.insert(Rectangle(x1, y1, x2, y2));
    rectsAll.insert(Rectangle(x1, y1, x2, y2));
    if (x1 >= subDataLimits.x1 && y1 >= subDataLimits.y1 && x2 <= subDataLimits.x2 && y2 <= subDataLimits.y2)
      rectsSub.insert(Rectangle(x1, y1, x2, y2));
    if (x1 <= subDataLimits.x2 && x2 >= subDataLimits.x1 && y1 <= subDataLimits.y2 && y2 >= subDataLimits.y1)
      rectsSubCollide.insert(Rectangle(x1, y1, x2, y2));
    });
  //S'assure de la bonne taille du QuadTree et de la profondeur
  REQUIRE(qt.size() == datasetSize);
  REQUIRE(qt.depth() == depth);

  SECTION("container") {
    //Vérifie que tous les rectangles sont bien présents (ni plus, ni moins)
    auto rects = qt.getAll();
    for (const auto& rect : rects)
    {
      auto found = rectsAll.find(rect);
      REQUIRE(found != rectsAll.end());
      rectsAll.erase(found);
    }
    REQUIRE(rectsAll.empty());

    //Vérifie que les rectangles inclus dans subDataLimits sont bien présents (ni plus, ni moins)
    rects = qt.findInscribed(subDataLimits);
    for (const auto& rect : rects)
    {
      auto found = rectsSub.find(rect);
      REQUIRE(found != rectsSub.end());
      rectsSub.erase(found);
    }
    REQUIRE(rectsSub.empty());

    //Vérifie que les rectangles en collision avec subDataLimits sont bien présents (ni plus, ni moins)
    rects = qt.findColliding(subDataLimits);
    for (const auto& rect : rects)
    {
      auto found = rectsSubCollide.find(rect);
      REQUIRE(found != rectsSubCollide.end());
      rectsSubCollide.erase(found);
    }
    REQUIRE(rectsSubCollide.empty());
  }

  SECTION("iterator") {
    //Vérifie que tous les rectangles sont bien présents (ni plus, ni moins)
    for (const auto& rect : qt)
    {
      auto found = rectsAll.find(rect);
      REQUIRE(found != rectsAll.end());
      rectsAll.erase(found);
    }
    REQUIRE(rectsAll.empty());

    //Vérifie que les rectangles inclus dans subDataLimits sont bien présents (ni plus, ni moins)
    for (auto it = qt.beginInscribed(subDataLimits); it != qt.end(); ++it)
    {
      auto found = rectsSub.find(*it);
      REQUIRE(found != rectsSub.end());
      rectsSub.erase(found);
    }
    REQUIRE(rectsSub.empty());

    //Vérifie que les rectangles en collision avec subDataLimits sont bien présents (ni plus, ni moins)
    for (auto it = qt.beginColliding(subDataLimits); it != qt.end(); ++it)
    {
      auto found = rectsSubCollide.find(*it);
      REQUIRE(found != rectsSubCollide.end());
      rectsSubCollide.erase(found);
    }
    REQUIRE(rectsSubCollide.empty());
  }

  //Vide le QuadTree et vérifie qu'il est bien vide
  qt.clear();
  REQUIRE(qt.empty());
  REQUIRE(qt.size() == 0);
  REQUIRE(qt.depth() == 1);
}

TEST_CASE("TQuadTree.3-QuadTree copy test", "[copy]") {
  QuadTree qt;
  //Crée un QuadTree avec 100 rectangles de taille 0.1x0.1 répartis aléatoirement sur une surface de 1x1
  std::random_device rd;
  std::default_random_engine dre(rd());
  std::uniform_real_distribution<float> urd(0.0f, 1.0f);
  for (size_t i = 0; i < 100; i++)
  {
    float x1, y1, x2, y2;
    x1 = urd(dre) * 0.9f;
    y1 = urd(dre) * 0.9f;
    x2 = x1 + 0.1f;
    y2 = y1 + 0.1f;
    qt.insert(Rectangle{ x1,y1,x2,y2 });
  }
  REQUIRE(qt.size() == 100);

  //Crée un copie de ce QuadTree
  QuadTree qt2(qt);
  REQUIRE(qt.size() == qt2.size());
  SECTION("container") {
    auto all = qt.getAll();
    auto all2 = qt2.getAll();
    //S'assure que les deux QuadTree ont les mêmes rectangles
    for (auto it = all.begin(), it2 = all2.begin(); it != all.end(); it++, it2++)
    {
      REQUIRE(*it == *it2);
    }
    //Vide le premier QuadTree puis s'assure que le deuxième n'a pas été affecté
    qt.clear();
    REQUIRE(qt.empty() == true);
    REQUIRE(qt.size() == 0);
    REQUIRE(qt2.size() == 100);
    all2 = qt2.getAll();
    REQUIRE(all2.size() == 100);
    for (auto it = all.begin(), it2 = all2.begin(); it != all.end(); it++, it2++)
    {
      REQUIRE(*it == *it2);
    }
  }

  SECTION("iterator") {
    std::list<Rectangle> rects;
    //S'assure que les deux QuadTree ont les mêmes rectangles
    for (auto it = qt.begin(), it2 = qt2.begin(); it != qt.end(); it++, it2++)
    {
      rects.push_back(*it);
      REQUIRE(*it == *it2);
    }
    //Vide le premier QuadTree puis s'assure que le deuxième n'a pas été affecté
    qt.clear();
    REQUIRE(qt.empty() == true);
    REQUIRE(qt.size() == 0);
    REQUIRE(qt2.size() == 100);
    auto it = rects.begin();
    auto it2 = qt2.begin();
    for (; it != rects.end(); it++, it2++)
    {
      REQUIRE(*it == *it2);
    }
  }
}

TEST_CASE("TQuadTree.4-QuadTree stress test", "[stress]") {
  //Lit un fichier de données de test et initialise un QuadTree avec ces données.
  //Stocke égalements les rectangles dans deux ensembles pour vérifier leur présence dans le QuadTree
  //L'ensemble rectsAll contient tous les rectangles, rectsSub contient ceux qui sont inclus dans subDataLimits
  QuadTree qt;
  size_t depth;
  size_t datasetSize;
  std::multiset<Rectangle> rectsAll;
  std::multiset<Rectangle> rectsSub;
  std::multiset<Rectangle> rectsSubCollide;
  readDataSet(depth, datasetSize, [&qt, &rectsAll, &rectsSub, &rectsSubCollide](float x1, float y1, float x2, float y2) {
    qt.insert(Rectangle(x1, y1, x2, y2));
    rectsAll.insert(Rectangle(x1, y1, x2, y2));
    if (x1 >= subDataLimits.x1 && y1 >= subDataLimits.y1 && x2 <= subDataLimits.x2 && y2 <= subDataLimits.y2)
      rectsSub.insert(Rectangle(x1, y1, x2, y2));
    if (x1 <= subDataLimits.x2 && x2 >= subDataLimits.x1 && y1 <= subDataLimits.y2 && y2 >= subDataLimits.y1)
      rectsSubCollide.insert(Rectangle(x1, y1, x2, y2));
    });
  //S'assure de la bonne taille du QuadTree et de la profondeur
  REQUIRE(qt.size() == datasetSize);
  REQUIRE(qt.depth() == depth);

  SECTION("container") {
    //Vérifie que tous les rectangles sont bien présents (ni plus, ni moins)
    auto all = qt.getAll();
    REQUIRE(all.size() == datasetSize);
    for (const auto& rect : all)
    {
      auto found = rectsAll.find(rect);
      REQUIRE(found != rectsAll.end());
      rectsAll.erase(found);
    }
    REQUIRE(rectsAll.empty());

    //Vérifie que les rectangles inclus dans subDataLimits sont bien présents (ni plus, ni moins)
    auto subdata = qt.findInscribed(subDataLimits);
    REQUIRE(subdata.size() == rectsSub.size());
    for (const auto& rect : subdata)
    {
      auto found = rectsSub.find(rect);
      REQUIRE(found != rectsSub.end());
      rectsSub.erase(found);
    }
    REQUIRE(rectsSub.empty());

    //Vérifie que les rectangles en collision avec subDataLimits sont bien présents (ni plus, ni moins)
    auto subdataCollide = qt.findColliding(subDataLimits);
    REQUIRE(subdataCollide.size() == rectsSubCollide.size());
    for (const auto& rect : subdataCollide)
    {
      auto found = rectsSubCollide.find(rect);
      REQUIRE(found != rectsSubCollide.end());
      rectsSubCollide.erase(found);
    }
    REQUIRE(rectsSubCollide.empty());
  }

  SECTION("iterator") {
    //Vérifie que tous les rectangles sont bien présents (ni plus, ni moins)
    for (const auto& rect : qt)
    {
      auto found = rectsAll.find(rect);
      REQUIRE(found != rectsAll.end());
      rectsAll.erase(found);
    }
    REQUIRE(rectsAll.empty());

    //Vérifie que les rectangles inclus dans subDataLimits sont bien présents (ni plus, ni moins)
    for (auto it = qt.beginInscribed(subDataLimits); it != qt.end(); ++it)
    {
      auto found = rectsSub.find(*it);
      REQUIRE(found != rectsSub.end());
      rectsSub.erase(found);
    }
    REQUIRE(rectsSub.empty());

    //Vérifie que les rectangles en collision avec subDataLimits sont bien présents (ni plus, ni moins)
    for (auto it = qt.beginColliding(subDataLimits); it != qt.end(); ++it)
    {
      auto found = rectsSubCollide.find(*it);
      REQUIRE(found != rectsSubCollide.end());
      rectsSubCollide.erase(found);
    }
    REQUIRE(rectsSubCollide.empty());
  }

  //Vide le QuadTree et vérifie qu'il est bien vide
  qt.clear();
  REQUIRE(qt.empty());
  REQUIRE(qt.size() == 0);
  REQUIRE(qt.depth() == 1);
}

TEST_CASE("TQuadTree.5-QuadTree performance test", "[performance]") {
  //Lit un fichier de données de test
  std::list<Rectangle> rectsAll;
  size_t depth;
  size_t datasetSize;
  readDataSet(depth, datasetSize, [&rectsAll](float x1, float y1, float x2, float y2) {
    rectsAll.push_back(Rectangle(x1, y1, x2, y2));
    });

  std::chrono::high_resolution_clock::time_point start, end;
  std::chrono::milliseconds insertionTime_ms;
  std::chrono::milliseconds findingTime_ms;
  std::chrono::milliseconds collidingTime_ms;

  //Benchmarke le temps d'insertion de tous les rectangles dans un QuadTree
#ifdef _DEBUG
  size_t memSize;
  _CrtMemState memStart, memEnd, memDiff;
  _CrtMemCheckpoint(&memStart);
#endif

  start = std::chrono::high_resolution_clock::now();
  QuadTree qt;
  for (const auto& rect : rectsAll)
  {
    qt.insert(rect);
  }
  end = std::chrono::high_resolution_clock::now();
#ifdef _DEBUG
  _CrtMemCheckpoint(&memEnd);
  _CrtMemDifference(&memDiff, &memStart, &memEnd);
  memSize = memDiff.lSizes[_NORMAL_BLOCK];
  #define MEMORY_USAGE_MESSAGE "Memory usage: " << memSize << " bytes\n"
#else
  #define MEMORY_USAGE_MESSAGE "Memory usage: not tracked in Release mode\n"
#endif

  insertionTime_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

  SECTION("container") {
    //Benchmarke le temps de recherche de tous les rectangles inclus dans subDataLimits
    start = std::chrono::high_resolution_clock::now();
    qt.findInscribed(subDataLimits);
    end = std::chrono::high_resolution_clock::now();
    findingTime_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    //Benchmarke le temps de recherche de tous les rectangles en collision avec subDataLimits
    start = std::chrono::high_resolution_clock::now();
    qt.findColliding(subDataLimits);
    end = std::chrono::high_resolution_clock::now();
    collidingTime_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    //Rapporte les résultats
    SUCCEED("Insertion time: " << insertionTime_ms.count() << " ms\n"
      "Finding time (container): " << findingTime_ms.count() << " ms\n"
      "Colliding time (container): " << collidingTime_ms.count() << " ms\n"
      MEMORY_USAGE_MESSAGE);
  }

  SECTION("iterator") {
    //Benchmarke le temps de recherche de tous les rectangles inclus dans subDataLimits (avec des itérateurs)
    start = std::chrono::high_resolution_clock::now();
    for (auto it = qt.beginInscribed(subDataLimits); it != qt.end(); ++it);
    end = std::chrono::high_resolution_clock::now();
    findingTime_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    //Benchmarke le temps de recherche de tous les rectangles en collision avec subDataLimits
    start = std::chrono::high_resolution_clock::now();
    for (auto it = qt.beginColliding(subDataLimits); it != qt.end(); ++it);
    end = std::chrono::high_resolution_clock::now();
    collidingTime_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    //Rapporte les résultats
    SUCCEED("Insertion time: " << insertionTime_ms.count() << " ms\n"
      "Finding time (iterator): " << findingTime_ms.count() << " ms\n"
      "Colliding time (iterator): " << collidingTime_ms.count() << " ms\n"
      MEMORY_USAGE_MESSAGE);
  }
}
