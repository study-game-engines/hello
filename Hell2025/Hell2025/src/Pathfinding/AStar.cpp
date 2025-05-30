#include "AStar.h"
//#include "Pathfinding.h"
#include "AStarMap.h"
#include "Timer.hpp"

void AStar::InitSearch(int startX, int startY, int destinationX, int destinationY) {
    ClearData();

    m_cells.resize(AStarMap::GetCellCount());
    m_mapWidth = AStarMap::GetMapWidth();
    m_mapHeight = AStarMap::GetMapHeight();

    if (!AStarMap::IsInBounds(startX, startY) ||
        !AStarMap::IsInBounds(destinationX, destinationY)) {
        return;
    }

    m_openList.AllocateSpace(m_mapWidth * m_mapHeight);
    m_openList.Clear();

    int idxStart = Index1D(startX, startY);
    int idxDestination = Index1D(destinationX, destinationY);

    m_start = &m_cells[idxStart];
    m_destination = &m_cells[idxDestination];
    m_current = m_start;
    m_start->g = 0;
    m_start->GetF(m_destination);
    m_openList.AddItem(m_start);
    m_searchInitilized = true;

    // Preprocess the cells
    for (int i = 0; i < m_cells.size(); i++) {
        int x = i % m_mapWidth;
        int y = i / m_mapWidth;
        int idx = Index1D(x, y);

        Cell& cell = m_cells[idx];
        cell.x = x;
        cell.y = y;
        cell.obstacle = AStarMap::IsCellObstacle(x, y);
        cell.g = 99999;
        cell.f = -1;
        cell.parent = nullptr;
        cell.neighbours.clear();

        // Precompute H
        int dx = std::abs(cell.x - m_destination->x);
        int dy = std::abs(cell.y - m_destination->y);
        cell.h = ORTHOGONAL_COST * (dx + dy) + (DIAGONAL_COST - 2 * ORTHOGONAL_COST) * std::min(dx, dy);
    }

    // initialize closed list to false
    m_closedFlags.assign(m_mapWidth * m_mapHeight, false);

    // Cache all neighbours
    for (int x = 0; x < m_mapWidth; x++) {
        for (int y = 0; y < m_mapHeight; y++) {
            FindNeighbours(x, y);
        }
    }
}

void AStar::ClearData() {
    m_closedFlags.clear();
    m_finalPath.clear();
    m_openList.Clear();
    m_intersectionPoints.clear();
    m_gridPathFound = false;
    m_smoothPathFound = false;
    m_searchInitilized = false;
    m_smoothSearchIndex = 0;
}

bool AStar::GridPathFound() {
    return m_gridPathFound;
}

bool AStar::SmoothPathFound() {
    return m_smoothPathFound;
}

bool AStar::SearchInitilized() {
    return m_searchInitilized;
}

void AStar::FindPath() {
    //Timer timer("RooPathFind");

    if (m_destination->obstacle) {
        std::cout << "FindPath() failed because destination was an obstacle\n";
        return;
    }
    if (m_openList.IsEmpty()) {
        std::cout << "FindPath() bailed early coz open list was empty\n";
        return;
    }
    if (m_gridPathFound) {
        std::cout << "FindPath() bailed early coz a path was already found\n";
        return;
    }

    //int cellsSearched = 0;
    while (!m_openList.IsEmpty())
    {
        m_current = m_openList.RemoveFirst();
        if (IsDestination(m_current)) {
            m_gridPathFound = true;
            BuildFinalPath();
            //std::cout << "cellsSearched: " << cellsSearched << "\n";
            //PrintPath();
            return;
        }

        //cellsSearched++;

        // Add current cell to closed list
        int idxCurrent = Index1D(m_current->x, m_current->y);
        m_closedFlags[idxCurrent] = true;

        for (Cell* neighbour : m_current->neighbours) {
            // Calculate G cost. Equal to parent G cost + 10 if orthogonal and + 14 if diagonal
            int new_g = IsOrthogonal(m_current, neighbour) ? m_current->g + ORTHOGONAL_COST : m_current->g + DIAGONAL_COST;

            if (IsInClosedList(neighbour) || m_openList.Contains(neighbour)) {
                // If new G is lower than currently stored value, update it and change parent to the current cell
                if (new_g < neighbour->g) {
                    neighbour->g = new_g;
                    neighbour->f = new_g + neighbour->GetH(m_destination);
                    neighbour->parent = m_current;
                }
            }
            else {
                neighbour->g = new_g;
                neighbour->f = new_g + neighbour->GetH(m_destination);
                neighbour->parent = m_current;

                if (!m_openList.Contains(neighbour)) {
                    m_openList.AddItem(neighbour);
                }
            }
        }
    }

    //if (!Pathfinding::SlowModeEnabled()) {
    ////    FindPath();
    //}
}

void AStar::PrintPath() {

    for (int y = 0; y < m_mapWidth; ++y) {
        for (int x = 0; x < m_mapHeight; ++x) {

            bool inPath = false;
            for (Cell* cell : m_finalPath) {
                if (cell->x == x && cell->y == y) {
                    inPath = true;
                    break;
                }
            }

            int idx = Index1D(x, y);

            if (x == m_start->x && y == m_start->y) {
                std::cout << "A";
            }
            else if (x == m_destination->x && y == m_destination->y) {
                std::cout << "B";
            }
            else if (inPath) {
                std::cout << ".";
            }
            else if (AStarMap::IsCellObstacle(x, y)) {
                std::cout << char(219);
            }
            else {
                std::cout << " ";
            }
        }
        std::cout << "\n";
    }
}

MinHeap& AStar::GetOpenList() {
    return m_openList;
}

std::vector<Cell*>& AStar::GetPathCellVersion() {
    return m_finalPath;
}

// Optimize me!!!!
std::vector<glm::ivec2> AStar::GetPath() {
    std::vector<glm::ivec2> path;
    for (Cell* cell : m_finalPath) {
        path.push_back(glm::ivec2(cell->x, cell->y));
    }
    return path;
}

bool AStar::IsDestination(Cell* cell) {
    return cell == m_destination;
}

void AStar::BuildFinalPath() {
    Cell* cell = m_destination;
    while (cell != m_start) {
        m_finalPath.push_back(cell);
        cell = cell->parent;
    }
    std::reverse(m_finalPath.begin(), m_finalPath.end());

    // Init smooth search
    m_smoothPathFound = false;
    m_smoothSearchIndex = 2;
    m_intersectionPoints.clear();
    glm::vec2 startPoint = glm::vec2(m_start->x + 0.5f, m_start->y + 0.5f);
    glm::vec2 endPoint = glm::vec2(m_destination->x + 0.5f, m_destination->y + 0.5f);
    m_intersectionPoints.push_back(startPoint);
    for (int j = 0; j < m_finalPath.size(); j++) {
        Cell* cell = m_finalPath[j];
        m_intersectionPoints.push_back(glm::vec2(cell->x, cell->y));
    }
    m_intersectionPoints.push_back(endPoint);
}

//void AStar::FindSmoothPath() {
//    if (!m_gridPathFound) {
//        return;
//    }
//    if (m_smoothSearchIndex >= m_intersectionPoints.size()) {
//        m_smoothPathFound = true;
//    }
//    if (m_smoothPathFound) {
//        return;
//    }
//    // Remove points with line of sight
//    glm::vec2 currentPosition = m_intersectionPoints[m_smoothSearchIndex];
//    glm::vec2 queryPosition = m_intersectionPoints[m_smoothSearchIndex - 2];
//    glm::vec2 intersectionPointOut;
//    if (Pathfinding::HasLineOfSight(currentPosition, queryPosition)) {
//        m_intersectionPoints.erase(m_intersectionPoints.begin() + m_smoothSearchIndex - 1);
//    }
//    else {
//        m_smoothSearchIndex++;
//    }
//    if (!Pathfinding::SlowModeEnabled()) {
//        FindSmoothPath();
//    }
//}

bool AStar::IsOrthogonal(Cell* cellA, Cell* cellB) {
    return (cellA->x == cellB->x || cellA->y == cellB->y);
}

bool AStar::IsInClosedList(Cell* cell) {
    int idx = Index1D(cell->x, cell->y);
    return m_closedFlags[idx];
}

void AStar::FindNeighbours(int x, int y) {
    int idx = Index1D(x, y);
    int idxN = Index1D(x, y - 1);
    int idxS = Index1D(x, y + 1);
    int idxE = Index1D(x - 1, y);
    int idxW = Index1D(x + 1, y);
    int idxNW = Index1D(x - 1, y - 1);
    int idxNE = Index1D(x + 1, y - 1);
    int idxSW = Index1D(x - 1, y + 1);
    int idxSE = Index1D(x + 1, y + 1);

    // Don't search for neighbours if they already exist
    if (m_cells[idx].neighbours.size() != 0) {
        return;
    }

    // North
    if (AStarMap::IsInBounds(x, y - 1) && !AStarMap::IsCellObstacle(x, y - 1)) {
        m_cells[idx].neighbours.push_back(&m_cells[idxN]);
    }
    // South
    if (AStarMap::IsInBounds(x, y + 1) && !AStarMap::IsCellObstacle(x, y + 1)) {
        m_cells[idx].neighbours.push_back(&m_cells[idxS]);
    }
    // West
    if (AStarMap::IsInBounds(x - 1, y) && !AStarMap::IsCellObstacle(x - 1, y)) {
        m_cells[idx].neighbours.push_back(&m_cells[idxE]);
    }
    // East
    if (AStarMap::IsInBounds(x + 1, y) && !AStarMap::IsCellObstacle(x + 1, y)) {
        m_cells[idx].neighbours.push_back(&m_cells[idxW]);
    }
    // Northwest
    if (AStarMap::IsInBounds(x - 1, y - 1) && !AStarMap::IsCellObstacle(x - 1, y - 1)) {
        m_cells[idx].neighbours.push_back(&m_cells[idxNW]);
    }
    // Northeast
    if (AStarMap::IsInBounds(x + 1, y - 1) && !AStarMap::IsCellObstacle(x + 1, y - 1)) {
        m_cells[idx].neighbours.push_back(&m_cells[idxNE]);
    }
    // Southwest
    if (AStarMap::IsInBounds(x - 1, y + 1) && !AStarMap::IsCellObstacle(x - 1, y + 1)) {
        m_cells[idx].neighbours.push_back(&m_cells[idxSW]);
    }
    // Southeast
    if (AStarMap::IsInBounds(x + 1, y + 1) && !AStarMap::IsCellObstacle(x + 1, y + 1)) {
        m_cells[idx].neighbours.push_back(&m_cells[idxSE]);
    }
    

}
