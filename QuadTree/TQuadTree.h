#pragma once
//Evidemment, il va falloir inclure les fichiers nécessaires pour que le code compile
#include <vector>
#include <memory>
//Vous n'avez pas le droit de modifier cette partie du code jusqu'à la ligne notée par le commentaire //Vous pouvez modifier le code ci-dessous
#include <concepts>
#include <stdexcept>
#include <algorithm>

/**
 * @brief Concept définissant les exigences pour les données pouvant être stockées dans un TQuadTree.
 *
 * Ce concept vérifie que le type T possède les méthodes x1(), y1(), x2() et y2() qui retournent des valeurs convertibles en float.
 * De plus, le type T doit être copiable et comparable pour l'égalité.
 *
 * @tparam T Le type des données à vérifier.
 *
 * Les exigences pour le type T sont les suivantes :
 * - T doit avoir une méthode x1() retournant un type convertible en float.
 * - T doit avoir une méthode y1() retournant un type convertible en float.
 * - T doit avoir une méthode x2() retournant un type convertible en float.
 * - T doit avoir une méthode y2() retournant un type convertible en float.
 * - T doit être copiable.
 * - T doit être comparable pour l'égalité.
 */
template<typename T>
concept QuadTreeData = requires(T t)
{
    { t.x1() } -> std::convertible_to<float>;
    { t.y1() } -> std::convertible_to<float>;
    { t.x2() } -> std::convertible_to<float>;
    { t.y2() } -> std::convertible_to<float>;
}&& std::copyable<T>&& std::equality_comparable<T>;

/**
 * @brief Structure définissant les limites d'une zone rectangulaire.
 */
struct SLimits
{
    float x1; ///< La coordonnée x du coin supérieur gauche.
    float y1; ///< La coordonnée y du coin supérieur gauche.
    float x2; ///< La coordonnée x du coin inférieur droit.
    float y2; ///< La coordonnée y du coin inférieur droit.

    bool operator==(const SLimits& other) const = default;
};

/**
 * @brief Classe de QuadTree.
 *
 * Cette classe permet de stocker des données de type T dans un QuadTree.
 *
 * @tparam T Le type des données à stocker.
 * T doit respecter le concept QuadTreeData.
 */
template <QuadTreeData T>
class TQuadTree
{
    //Vous pouvez modifier le code ci-dessous
    //Attention à ne pas modifier les signatures des fonctions et des méthodes qui sont déjà présentes
public:
    using container = std::vector<T>;

    /**
      * @brief Itérateur pour parcourir les éléments du QuadTree.
      */
    class iterator {
    public:
        //Ces typedefs sont nécessaires pour que l'itérateur soit reconnu comme un itérateur d'entrée
        //par les algorithmes de la STL. Vous ne devriez pas les modifier.
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = T*;
        using reference = T&;
        using iterator_category = std::input_iterator_tag;

    private:
        typename container::const_iterator _current;
        typename container::const_iterator _end;
        std::shared_ptr<container> m_data;
        std::size_t m_index = 0;

    public:
        /**
         * @brief Constructeur par défaut de l'itérateur.
         *
         * Evidemment, il va falloir compléter cette fonction pour qu'elle initialise correctement l'itérateur.
         * Peut-être que l'itérateur devrait être initialisé à la fin de la liste de données.
         * Peut-etre qu'on peut ajouter d'autres constructeurs pour initialiser l'itérateur à un endroit spécifique.
         */
        iterator() = default;
        iterator(std::shared_ptr<container> data, std::size_t idx)
            : m_data(std::move(data)), m_index(idx)
        {
        }

        /**
         * @brief Opérateur de comparaison d'égalité.
         *
         * @returns true si les deux itérateurs sont à la même position.
         */
        bool operator==(const iterator& other) const
        {
            //Evidemment, il va falloir compléter cette fonction pour qu'elle retourne true si les deux itérateurs sont à la même position

            // Cas "end" => deux m_data nuls
            if (!m_data && !other.m_data) {
                return true;
            }

            if (!m_data || !other.m_data) {
                return false;
            }

            // Sinon, compare vecteur + index
            return (m_data == other.m_data) && (m_index == other.m_index);
        }

        bool operator!=(const iterator& other) const
        {
            return !(*this == other);
        }

        /**
         * @brief Opérateur de pré-incrémentation de l'itérateur.
         *
         * @returns l'itérateur courant avancé vers la prochaine position valide.
         */
        iterator& operator++()
        {
            //Evidemment, il va falloir compléter cette fonction pour qu'elle avance l'itérateur à la prochaine position valide
            if (!m_data) {
                // Already end
                return *this;
            }
            ++m_index;
            if (m_index >= m_data->size()) {
                // Devenir "end"
                m_data.reset();
                m_index = 0;
            }
            return *this;
        }

        /**
         * @brief Opérateur de post-incrémentation de l'itérateur.
         *
         * @param int Un paramètre fictif pour distinguer l'opérateur de pré-incrémentation de celui-ci.
         *
         * @returns l'itérateur courant avant d'être avancé vers la prochaine position valide.
         */
        iterator operator++(int)
        {
            //Evidemment, il va falloir compléter cette fonction pour qu'elle retourne l'itérateur courant avant d'être avancé
            iterator temp = *this;
            ++(*this);
            return temp;
        }

        /**
         * @brief Opérateur de déréférencement de l'itérateur.
         *
         * @returns une référence vers l'élément pointé par l'itérateur.
         */
        const T& operator*() const
        {
            if (!m_data) {
                throw std::logic_error("Dereferencing end iterator");
            }
            return (*m_data)[m_index];
        }

        T* operator->()
        {
            return &(this->operator*());
        }
    };

private:
    static constexpr std::size_t CAPACITY = 1;  ///< Nombre max. d'éléments avant subdivision
    SLimits _limits;
    container m_data;
    std::unique_ptr<TQuadTree> m_children[4] = { nullptr };

    /**
     * @brief Epsilon pour accepter un léger dépassement/arrondi dans isFullyInside.
     *        Ajustez cette valeur en cas de sous/sur-subdivision.
     */
    static constexpr float EPS = 1e-7f;

    /**
     * @brief Vérifie si 'inner' est entièrement à l'intérieur de 'outer',
     *        avec une petite marge EPS pour gérer les flottants.
     */
    static bool isFullyInside(const SLimits& itemBounds, const SLimits& searchLimits) {
        return itemBounds.x1 >= searchLimits.x1 &&
            itemBounds.y1 >= searchLimits.y1 &&
            itemBounds.x2 <= searchLimits.x2 &&
            itemBounds.y2 <= searchLimits.y2;
    }

    /**
     * @brief Vérifie si a et b se recouvrent (overlap).
     */
    static bool overlap(const SLimits& a, const SLimits& b) noexcept {
        return !(b.x1 > a.x2 || b.x2 < a.x1 || b.y1 > a.y2 || b.y2 < a.y1);
    }

    /**
     * @brief Récupère les limites d'un T (Rectangle, etc.).
     */
    static SLimits boundsOf(const T& t) noexcept {
        return { t.x1(), t.y1(), t.x2(), t.y2() };
    }

    /**
     * @brief Indique si le rectangle r peut s'insérer entièrement dans un des 4 enfants (si on subdivise).
     */
    bool canFitChild(const SLimits& r) const noexcept
    {
        // Calcul midX, midY en double pour limiter les écarts
        float midX = (_limits.x1 + _limits.x2) * 0.5f;
        float midY = (_limits.y1 + _limits.y2) * 0.5f;

        // Quadrants
        SLimits child[4] = {
          { _limits.x1, _limits.y1, midX,       midY       }, // NW
          { midX,        _limits.y1, _limits.x2, midY       }, // NE
          { _limits.x1, midY,        midX,        _limits.y2}, // SW
          { midX,        midY,        _limits.x2, _limits.y2}, // SE
        };
        for (int i = 0; i < 4; i++) {
            if (isFullyInside(r, child[i])) {
                return true;
            }
        }
        return false;
    }


    /**
     * @brief Subdivise le nœud courant en 4, et réaffecte si possible les éléments dans les enfants.
     */
    void subdivide()
    {
        // Déjà subdivisé ?
        if (m_children[0]) {
            return;
        }

        // Calcul en double
        float midX = (_limits.x1 + _limits.x2) * 0.5f;
        float midY = (_limits.y1 + _limits.y2) * 0.5f;

        // Crée les 4 enfants
        m_children[0] = std::make_unique<TQuadTree>(SLimits{ _limits.x1, _limits.y1, midX,       midY }); // NW
        m_children[1] = std::make_unique<TQuadTree>(SLimits{ midX,        _limits.y1, _limits.x2, midY }); // NE
        m_children[2] = std::make_unique<TQuadTree>(SLimits{ _limits.x1, midY,        midX,        _limits.y2 }); // SW
        m_children[3] = std::make_unique<TQuadTree>(SLimits{ midX,        midY,        _limits.x2, _limits.y2 }); // SE

        // Tente de redescendre les éléments existants
        container remain;
        remain.reserve(m_data.size());

        for (const auto& item : m_data) {
            SLimits r = boundsOf(item);
            bool placed = false;
            for (int i = 0; i < 4; ++i) {
                if (isFullyInside(r, m_children[i]->_limits)) {
                    m_children[i]->insert(item);
                    placed = true;
                    break;
                }
            }
            if (!placed) {
                remain.push_back(item);
            }
        }

        // on swap
        m_data.swap(remain);
    }

    /**
     * @brief Copie récursive d'un autre TQuadTree.
     */
    void copyFrom(const TQuadTree& other)
    {
        _limits = other._limits;
        m_data = other.m_data;
        for (int i = 0; i < 4; i++) {
            if (other.m_children[i]) {
                m_children[i] = std::make_unique<TQuadTree>(*other.m_children[i]);
            }
            else {
                m_children[i].reset();
            }
        }
    }

public:
    /**
     * @brief Constructeur de la classe TQuadTree.
     *
     * @param limits Les limites géométriques du QuadTree.
     */
    TQuadTree(const SLimits& limits = { 0.0f,0.0f,1.0f,1.0f })
        : _limits(limits)
    {
        //Evidemment, il va falloir compléter ce constructeur pour qu'il initialise correctement votre TQuadTree
    }

    /**
     * @brief Constructeur de copie.
     */
    TQuadTree(const TQuadTree& other)
    {
        copyFrom(other);
    }

    /**
     * @brief Opérateur d'affectation par copie.
     */
    TQuadTree& operator=(const TQuadTree& other)
    {
        if (this != &other) {
            clear();
            copyFrom(other);
        }
        return *this;
    }

    /**
     * @brief Retourne les limites géométriques de ce QuadTree
     */
    SLimits limits() const
    {
        //Evidemment, il va falloir compléter cette fonction pour qu'elle retourne les limites géométriques de ce QuadTree
        return _limits;
    }

    /**
     * @brief Vérifie si le QuadTree est vide.
     *
     * Cette fonction vérifie si le QuadTree ne contient aucun élément de données, y compris dans la descendance.
     *
     * @return true si le QuadTree est vide, false sinon.
     */
    bool empty() const
    {
        //Evidemment, il va falloir compléter cette fonction pour qu'elle retourne true uniquement si le QuadTree est vide
        return (size() == 0);
    }

    /**
     * @brief Retourne la profondeur maximale du QuadTree.
     *
     * La profondeur maximale du QuadTree correspond nombre maximal de niveaux de descendants.
     */
    size_t depth() const
    {
        //Evidemment, il va falloir compléter cette fonction pour qu'elle retourne la profondeur maximale du QuadTree
        if (!m_children[0]) {
            return 1;
        }
        // 1 + max(enfants)
        std::size_t maxD = 0;
        for (int i = 0; i < 4; i++) {
            if (m_children[i]) {
                std::size_t d = m_children[i]->depth();
                if (d > maxD) {
                    maxD = d;
                }
            }
        }
        return 1 + maxD;
    }

    /**
     * @brief Retourne le nombre d'éléments stockés dans le QuadTree.
     *
     * Cette fonction retourne le nombre d'éléments stockés dans le QuadTree en incluant toute la descendance.
     *
     * @return Le nombre d'éléments stockés dans le QuadTree.
     */
    size_t size() const
    {
        //Evidemment, il va falloir compléter cette fonction pour qu'elle retourne le nombre d'éléments stockés dans le QuadTree
        std::size_t s = m_data.size();
        for (int i = 0; i < 4; i++) {
            if (m_children[i]) {
                s += m_children[i]->size();
            }
        }
        return s;
    }

    /**
  * @brief Insère un élément dans le QuadTree.
  *
  * Cette fonction insère l'élément donné dans le QuadTree. Si le nœud courant a des enfants,
  * elle essaie d'insérer l'élément dans un enfant approprié. Sinon, elle stocke l'élément dans le nœud courant.
  *
  * Si le nombre d'éléments dépasse la capacité ou si un élément peut être entièrement contenu dans un enfant,
  * le nœud est subdivisé.
  *
  * @param t L'élément à insérer.
  * @throws std::domain_error Si l'élément est en dehors des limites du QuadTree.
  */
    void insert(const T& t)
    {
        SLimits r = boundsOf(t);

        // Vérifie si l'élément est dans les limites du QuadTree
        if (!isFullyInside(r, _limits)) {
            throw std::domain_error("Object out of quadtree bounds");
        }

        // Si le nœud a déjà des enfants, tente d'insérer dans un enfant
        if (m_children[0]) {
            for (int i = 0; i < 4; i++) {
                if (isFullyInside(r, m_children[i]->_limits)) {
                    m_children[i]->insert(t);
                    return;
                }
            }
            // Si aucun enfant ne convient, stocke dans ce nœud
            m_data.push_back(t);
            return;
        }

        // Pas d'enfants, ajoute l'élément dans ce nœud
        m_data.push_back(t);

        // Subdivise si nécessaire (capacité dépassée ou l'élément peut rentrer dans un enfant)
        if (m_data.size() > CAPACITY || canFitChild(r)) {
            subdivide();
        }
    }


    /**
     * @brief Vide le QuadTree.
     *
     * Cette fonction vide le QuadTree de toutes ses données.
     */
    void clear()
    {
        //Evidemment, il va falloir compléter cette fonction pour qu'elle vide le QuadTree
        m_data.clear();
        for (auto& child : m_children) {
            child.reset();
        }
    }

    void remove(const T& t)
    {
        //Evidemment, il va falloir compléter cette fonction pour qu'elle retire l'élément du QuadTree
        SLimits r = boundsOf(t);
        // Si on a des enfants, on voit si l'élément peut y être
        if (m_children[0]) {
            for (int i = 0; i < 4; i++) {
                if (isFullyInside(r, m_children[i]->_limits)) {
                    m_children[i]->remove(t);
                    return;
                }
            }
        }
        // Sinon, on le retire de ce nœud localement
        auto it = std::find(m_data.begin(), m_data.end(), t);
        if (it != m_data.end()) {
            m_data.erase(it);
        }

    }

    /**
 * @brief Récupère tous les éléments stockés dans le QuadTree.
 *
 * Cette fonction retourne une liste de tous les éléments stockés dans le QuadTree,
 * y compris ceux stockés dans les enfants.
 *
 * @return Une liste de tous les éléments stockés dans le QuadTree.
 */
    container getAll() const
    {
        container result;
        result.reserve(size()); // Préalloue l'espace pour optimiser les insertions

        // Ajoute les éléments de l'objet courant
        result.insert(result.end(), m_data.begin(), m_data.end());

        // Parcourt les enfants et récupère leurs éléments récursivement
        for (int i = 0; i < 4; i++) {
            if (m_children[i]) {
                container childResults = m_children[i]->getAll();
                result.insert(result.end(), childResults.begin(), childResults.end());
            }
        }

        return result;
    }

    /**
     * @brief Trouve les éléments dans une zone spécifiée.
     *
     * Cette fonction recherche et retourne une liste de tous les éléments stockés dans le QuadTree
     * qui se trouvent totalement inclus dans la zone spécifiée par limits.
     *
     * @param limits Les limites de la zone de recherche.
     * @return Une liste de tous les éléments trouvés dans la zone spécifiée.
     */
     /**
  * @brief Trouve les éléments totalement inclus dans une zone spécifiée.
  *
  * Cette fonction recherche et retourne une liste de tous les éléments stockés dans le QuadTree
  * qui sont totalement inclus dans la zone spécifiée par limits.
  *
  * @param limits Les limites de la zone de recherche.
  * @return Une liste de tous les éléments trouvés dans la zone spécifiée.
  */
    container findInscribed(const SLimits& limits) const
    {
        container result;

        // Vérifie si l'objet courant chevauche la zone de recherche
        if (!overlap(_limits, limits)) {
            return result; // Aucun chevauchement
        }

        // Parcourt les éléments pour trouver ceux totalement inclus
        for (const auto& item : m_data) {
            if (isFullyInside(boundsOf(item), limits)) {
                result.push_back(item);
            }
        }


        // Explore récursivement les enfants s'ils existent
        for (const auto& child : m_children) {
            if (child) {
                container childResults = child->findInscribed(limits);
                result.insert(result.end(), childResults.begin(), childResults.end());
            }
        }


        return result;
    }

    /**
     * @brief Trouve les éléments dans une zone spécifiée.
     *
     * Cette fonction recherche et retourne une liste de tous les éléments stockés dans le QuadTree
     * qui sont en collision avec la zone spécifiée par limits.
     *
     * @param limits Les limites de la zone de recherche.
     * @return Une liste de tous les éléments trouvés dans la zone spécifiée.
     */
    container findColliding(const SLimits& limits) const
    {
        //Evidemment, il va falloir compléter cette fonction pour qu'elle retourne tous les éléments en collision avec la zone spécifiée
        container result;

        // Vérifie si l'objet courant chevauche la zone de recherche
        if (!overlap(_limits, limits)) {
            return result; // Aucun chevauchement
        }

        // Parcourt les éléments de la structure pour trouver ceux qui se chevauchent
        result.reserve(m_data.size());
        for (const auto& item : m_data) {
            if (overlap(boundsOf(item), limits)) {
                result.push_back(item);
            }
        }

        // Explore les enfants récursivement s'ils existent
        for (const auto& child : m_children) {
            if (child) {
                container childResults = child->findColliding(limits);
                result.insert(result.end(), childResults.begin(), childResults.end());
            }
        }

        return result;
    }

    /**
     * @brief Itérateur sur tous les éléments.
     */
    iterator begin()
    {
        // On fabrique un vecteur contenant tout
        auto data = std::make_shared<container>(getAll());
        return iterator(data, 0);
    }

    /**
     * @brief Itérateur sur les éléments qui collisionnent 'limits'.
     */
    iterator beginColliding(const SLimits& limits)
    {
        auto data = std::make_shared<container>(findColliding(limits));
        return iterator(data, 0);
    }

    /**
     * @brief Itérateur sur les éléments entièrement inclus dans 'limits'.
     */
    iterator beginInscribed(const SLimits& limits)
    {
        auto data = std::make_shared<container>(findInscribed(limits));
        return iterator(data, 0);
    }

    /**
     * @brief Itérateur de fin (sentinelle).
     */
    iterator end()
    {
        return {};
    }

};