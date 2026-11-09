// PA2 Basic: exact tiling, GNU C++11. No user-defined struct or class.
// Features: rotations/reflections, normalization, deduplication,
// recursive backtracking, most-constrained-cell search, region-area pruning.
#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <climits>
using namespace std;

typedef vector<pair<int,int>> Shape;
int H, W, P;
vector<int> board;                 // -1 = obstacle, 0 = empty, ID = covered
vector<int> sizes;
vector<vector<Shape>> shapes;
vector<bool> used;
vector<int> placementPiece;        // placement index -> 0-based piece ID
vector<vector<int>> placementCells;
vector<vector<int>> atCell;       // cell index -> possible placement indices

Shape normalized(Shape s) {
    int mr = INT_MAX, mc = INT_MAX;
    for (auto p : s) { mr = min(mr, p.first); mc = min(mc, p.second); }
    for (auto &p : s) { p.first -= mr; p.second -= mc; }
    sort(s.begin(), s.end());
    return s;
}

vector<Shape> uniqueOrientations(const Shape &original) {
    vector<Shape> result;
    for (int mirror = 0; mirror < 2; ++mirror) {
        Shape s = original;
        if (mirror) for (auto &p : s) p.second = -p.second;
        for (int rot = 0; rot < 4; ++rot) {
            Shape t = normalized(s);
            if (find(result.begin(), result.end(), t) == result.end())
                result.push_back(t);
            for (auto &p : s) {
                int r = p.first, c = p.second;
                p = make_pair(c, -r);
            }
        }
    }
    return result;
}

// Precompute placements; a placement contains only actual X cells.
void buildPlacements() {
    atCell.assign(H * W, vector<int>());
    for (int id = 0; id < P; ++id) {
        for (const Shape &s : shapes[id]) {
            int maxR = 0, maxC = 0;
            for (auto p : s) { maxR = max(maxR, p.first); maxC = max(maxC, p.second); }
            for (int top = 0; top + maxR < H; ++top) {
                for (int left = 0; left + maxC < W; ++left) {
                    vector<int> cells;
                    bool valid = true;
                    for (auto p : s) {
                        int index = (top + p.first) * W + left + p.second;
                        if (board[index] == -1) { valid = false; break; }
                        cells.push_back(index);
                    }
                    if (!valid) continue;
                    int k = (int)placementPiece.size();
                    placementPiece.push_back(id);
                    placementCells.push_back(cells);
                    for (int index : cells) atCell[index].push_back(k);
                }
            }
        }
    }
}

bool canPlace(int k) {
    if (used[placementPiece[k]]) return false;
    for (int index : placementCells[k])
        if (board[index] != 0) return false;
    return true;
}

// Every connected empty region must have an area expressible as
// a subset sum of the remaining piece sizes (necessary, not sufficient).
bool regionPrune() {
    bool possible[65] = {};
    possible[0] = true;
    int minSize = INT_MAX;
    for (int i = 0; i < P; ++i) if (!used[i]) {
        minSize = min(minSize, sizes[i]);
        for (int area = 64; area >= sizes[i]; --area)
            if (possible[area - sizes[i]]) possible[area] = true;
    }
    vector<bool> seen(H * W, false);
    for (int start = 0; start < H*W; ++start) {
        if (board[start] != 0 || seen[start]) continue;
        vector<int> queue(1, start);
        seen[start] = true;
        for (int j = 0; j < (int)queue.size(); ++j) {
            int x = queue[j], r = x/W, c = x%W;
            int neighbors[4] = {r>0?x-W:-1, r+1<H?x+W:-1,
                                c>0?x-1:-1, c+1<W?x+1:-1};
            for (int next : neighbors) if (next>=0 && board[next]==0 && !seen[next]) {
                seen[next] = true;
                queue.push_back(next);
            }
        }
        int area = (int)queue.size();
        if (area < minSize || !possible[area]) return false;
    }
    return true;
}

bool dfs(int placed) {
    if (placed == P) return true;
    if (!regionPrune()) return false;

    // Most-constrained empty cell: fewest currently legal placements.
    int bestCell = -1, bestCount = INT_MAX;
    for (int cell = 0; cell < H*W; ++cell) if (board[cell] == 0) {
        int count = 0;
        for (int k : atCell[cell]) if (canPlace(k)) ++count;
        if (count == 0) return false;
        if (count < bestCount) {
            bestCount = count;
            bestCell = cell;
            if (count == 1) break;
        }
    }
    if (bestCell == -1) return false; // Should be unreachable with valid input.

    for (int k : atCell[bestCell]) {
        if (!canPlace(k)) continue;
        int id = placementPiece[k];
        used[id] = true;
        for (int cell : placementCells[k]) board[cell] = id + 1;
        if (dfs(placed+1)) return true;
        for (int cell : placementCells[k]) board[cell] = 0;
        used[id] = false;
    }
    return false;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(NULL);
    if (!(cin >> H >> W >> P)) return 0;
    board.resize(H*W);
    int area = 0, total = 0;
    for (int r = 0; r < H; ++r) for (int c = 0; c < W; ++c) {
        char x; cin >> x;
        if (x == '#') board[r*W+c] = -1;
        else { board[r*W+c] = 0; ++area; }
    }
    sizes.resize(P);
    shapes.resize(P);
    used.assign(P, false);
    for (int id = 0; id < P; ++id) {
        int h, w; cin >> h >> w;
        Shape original;
        for (int r = 0; r < h; ++r) for (int c = 0; c < w; ++c) {
            char x; cin >> x;
            if (x == 'X') original.push_back(make_pair(r,c));
        }
        sizes[id] = (int)original.size();
        total += sizes[id];
        shapes[id] = uniqueOrientations(original);
    }
    if (area != total) { cout << "No solution\n"; return 0; }
    buildPlacements();
    if (!dfs(0)) { cout << "No solution\n"; return 0; }
    cout << "Possible\nUsed pieces: " << P << '\n';
    for (int r = 0; r < H; ++r) {
        for (int c = 0; c < W; ++c) {
            if (c) cout << ' ';
            int x = board[r*W+c];
            if (x == -1) cout << '#'; else cout << x;
        }
        cout << '\n';
    }
    return 0;
}
