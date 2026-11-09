// INTENTIONALLY INCORRECT BONUS PRACTICE DRAFT (not a historical record).
// BUG: stop at the first complete solution rather than global minimum.
// All other transformations, placement checks and pruning remain valid.
#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <climits>
using namespace std;

typedef vector<pair<int,int>> Shape;
int H,W,P,usable;
vector<int> board, bestBoard, sizes;
vector<vector<Shape>> shapes;
vector<bool> used;
vector<int> placementPiece;
vector<vector<int>> placementCells, atCell;
int bestK;

Shape normalize(Shape s) {
    int mr=INT_MAX, mc=INT_MAX;
    for (auto p:s) { mr=min(mr,p.first); mc=min(mc,p.second); }
    for (auto &p:s) { p.first-=mr; p.second-=mc; }
    sort(s.begin(),s.end());
    return s;
}
vector<Shape> allOrientations(Shape original) {
    vector<Shape> answer;
    for (int flip=0;flip<2;++flip) {
        Shape s=original;
        if (flip) for (auto &p:s) p.second=-p.second;
        for (int rot=0;rot<4;++rot) {
            Shape t=normalize(s);
            if (find(answer.begin(),answer.end(),t)==answer.end()) answer.push_back(t);
            for (auto &p:s) {
                int r=p.first,c=p.second;
                p=make_pair(c,-r);
            }
        }
    }
    sort(answer.begin(),answer.end()); // canonical ordering to compare piece shapes
    return answer;
}
void buildPlacements() {
    atCell.assign(H*W,vector<int>());
    // Large pieces first to obtain good upper bounds earlier.
    vector<int> order;
    for (int i=0;i<P;++i) order.push_back(i);
    sort(order.begin(),order.end(),[](int a,int b){
        if(sizes[a]!=sizes[b]) return sizes[a]<sizes[b]; // BUGGY search order, small pieces first
        return a<b;
    });
    for (int id:order) for (const Shape &s:shapes[id]) {
        int maxR=0,maxC=0;
        for (auto p:s) { maxR=max(maxR,p.first); maxC=max(maxC,p.second); }
        for (int top=0;top+maxR<H;++top) for(int left=0;left+maxC<W;++left) {
            vector<int> cells;
            bool ok=true;
            for (auto p:s) {
                int pos=(top+p.first)*W+left+p.second;
                if(board[pos]==-1) {ok=false;break;}
                cells.push_back(pos);
            }
            if(!ok) continue;
            int k=(int)placementPiece.size();
            placementPiece.push_back(id);
            placementCells.push_back(cells);
            for(int pos:cells) atCell[pos].push_back(k);
        }
    }
}
bool canPlace(int k) {
    int id=placementPiece[k];
    if(used[id]) return false;
    // Equivalent unused pieces are interchangeable; try smallest ID first.
    for(int other=0;other<id;++other)
        if(!used[other] && shapes[other]==shapes[id]) return false;
    for (int pos:placementCells[k]) if(board[pos]!=0) return false;
    return true;
}
// Return minimum number of currently unused pieces with exact total area 'remaining'.
// This is a LOWER BOUND: it ignores geometry.
int minPiecesByArea(int remaining, vector<bool> &possible) {
    const int INF=1000;
    vector<int> dp(65,INF);
    dp[0]=0;
    for(int i=0;i<P;++i) if(!used[i]) {
        int s=sizes[i];
        for(int a=64;a>=s;--a)
            if(dp[a-s]!=INF) dp[a]=min(dp[a],dp[a-s]+1);
    }
    possible.assign(65,false);
    for(int a=0;a<=64;++a) if(dp[a]!=INF) possible[a]=true;
    return dp[remaining];
}
bool regionPossible(const vector<bool> &possible) {
    vector<bool> seen(H*W,false);
    for (int start=0;start<H*W;++start) {
        if(board[start]!=0 || seen[start]) continue;
        vector<int> q(1,start);
        seen[start]=true;
        for(int j=0;j<(int)q.size();++j) {
            int x=q[j],r=x/W,c=x%W;
            int n[4]={r?x-W:-1, r+1<H?x+W:-1,
                       c?x-1:-1, c+1<W?x+1:-1};
            for(int y:n) if(y>=0 && board[y]==0 && !seen[y]) {
                seen[y]=true;
                q.push_back(y);
            }
        }
        if(!possible[q.size()]) return false;
    }
    return true;
}
void dfs(int placed,int remaining) {
    if(bestK<=P) return; // WRONG: one valid tiling is not necessarily optimal
    if(remaining==0) {
        if(placed<bestK) {bestK=placed;bestBoard=board;}
        return;
    }
    if(placed>=bestK) return;
    vector<bool> possible;
    int lower=minPiecesByArea(remaining,possible);
    if(lower>=1000 || placed+lower>=bestK) return;
    if(!regionPossible(possible)) return;

    int target=-1,bestCount=INT_MAX;
    for(int cell=0;cell<H*W;++cell) if(board[cell]==0) {
        int count=0;
        for(int k:atCell[cell]) if(canPlace(k)) ++count;
        if(count==0) return;
        if(count<bestCount) {
            target=cell;bestCount=count;
            if(count==1) break;
        }
    }
    for(int k:atCell[target]) {
        if(!canPlace(k)) continue;
        int id=placementPiece[k];
        used[id]=true;
        for(int pos:placementCells[k]) board[pos]=id+1;
        dfs(placed+1,remaining-sizes[id]);
        for(int pos:placementCells[k]) board[pos]=0;
        used[id]=false;
    }
}
int main() {
    ios::sync_with_stdio(false); cin.tie(NULL);
    if(!(cin>>H>>W>>P)) return 0;
    board.resize(H*W);
    usable=0;
    for(int r=0;r<H;++r) for(int c=0;c<W;++c) {
        char x;cin>>x;
        if(x=='#') board[r*W+c]=-1;
        else {board[r*W+c]=0;++usable;}
    }
    shapes.resize(P);sizes.resize(P);used.assign(P,false);
    for(int id=0;id<P;++id) {
        int h,w;cin>>h>>w;
        Shape s;
        for(int r=0;r<h;++r) for(int c=0;c<w;++c) {
            char x;cin>>x;
            if(x=='X') s.push_back(make_pair(r,c));
        }
        sizes[id]=(int)s.size();
        shapes[id]=allOrientations(s);
    }
    buildPlacements();
    bestK=P+1;
    dfs(0,usable);
    if(bestK==P+1) {cout<<"No solution\n";return 0;}
    cout<<"Possible\nUsed pieces: "<<bestK<<'\n';
    for(int r=0;r<H;++r) {
        for(int c=0;c<W;++c) {
            if(c) cout<<' ';
            int x=bestBoard[r*W+c];
            if(x==-1) cout<<'#';else cout<<x;
        }
        cout<<'\n';
    }
}
