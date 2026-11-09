// INTENTIONALLY INCORRECT PRACTICE DRAFT (not a historical record).
// BUG: only aligns the shape bounding-box origin with the first empty cell.
// The shape X-cell that must cover that empty cell might not be at (0,0).
#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
using namespace std;
using Shape = vector<pair<int,int>>;
int H,W,P;
vector<vector<int>> board;
vector<vector<Shape>> orientations;
vector<bool> used;

Shape normalize(Shape s) {
    int minR = 100, minC = 100;
    for (auto p : s) { minR = min(minR,p.first); minC = min(minC,p.second); }
    for (auto &p : s) { p.first -= minR; p.second -= minC; }
    sort(s.begin(),s.end());
    return s;
}
vector<Shape> getOrientations(Shape original) {
    vector<Shape> result;
    for (int flip = 0; flip < 2; ++flip) {
        Shape s = original;
        if (flip) for (auto &p : s) p.second = -p.second;
        for (int rotation = 0; rotation < 4; ++rotation) {
            Shape normalized = normalize(s);
            if (find(result.begin(),result.end(),normalized) == result.end())
                result.push_back(normalized);
            for (auto &p : s) {
                int r=p.first,c=p.second;
                p={c,-r};
            }
        }
    }
    return result;
}
bool dfs(int placed) {
    int r=-1,c=-1;
    for (int i=0;i<H && r<0;++i)
        for (int j=0;j<W;++j)
            if (board[i][j]==0) { r=i;c=j;break; }
    if (r<0) return placed==P;
    for (int id=0;id<P;++id) {
        if (used[id]) continue;
        for (const Shape &shape : orientations[id]) {
            for (int onlyOneAttempt=0;onlyOneAttempt<1;++onlyOneAttempt) {
                int top=r,left=c; // WRONG: skips alternative occupied-cell anchors
                bool ok=true;
                for (auto p : shape) {
                    int rr=top+p.first, cc=left+p.second;
                    if (rr<0||rr>=H||cc<0||cc>=W||board[rr][cc]!=0)
                        {ok=false;break;}
                }
                if (!ok) continue;
                for (auto p : shape) board[top+p.first][left+p.second]=id+1;
                used[id]=true;
                if (dfs(placed+1)) return true;
                used[id]=false;
                for (auto p : shape) board[top+p.first][left+p.second]=0;
            }
        }
    }
    return false;
}
int main() {
    cin >> H >> W >> P;
    board.assign(H,vector<int>(W,0));
    int area=0,total=0;
    for (int r=0;r<H;++r)
        for (int c=0;c<W;++c) {
            char ch;cin>>ch;
            if (ch=='#') board[r][c]=-1;
            else ++area;
        }
    orientations.resize(P);used.assign(P,false);
    for (int i=0;i<P;++i) {
        int h,w;cin>>h>>w;
        Shape s;
        for (int r=0;r<h;++r)
            for (int c=0;c<w;++c) {
                char ch;cin>>ch;
                if (ch=='X') {s.push_back({r,c});++total;}
            }
        orientations[i]=getOrientations(s);
    }
    if (area!=total||!dfs(0)) {cout<<"No solution\n";return 0;}
    cout<<"Possible\nUsed pieces: "<<P<<'\n';
    for (int r=0;r<H;++r) {
        for (int c=0;c<W;++c) {
            if(c)cout<<' ';
            if(board[r][c]<0)cout<<'#'; else cout<<board[r][c];
        }
        cout<<'\n';
    }
}
