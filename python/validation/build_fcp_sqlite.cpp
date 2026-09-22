#include <bits/stdc++.h>
#include <sqlite3.h>
using namespace std;
struct OrderInfo { int32_t customer=0; int16_t order_number=0; bool valid=false; };
struct Agg { int32_t purchase=0,reorder=0; int16_t first_order=32767,last_order=0; int32_t cart_sum=0,cart_n=0; };
static inline uint64_t splitmix64(uint64_t x){ x += 0x9e3779b97f4a7c15ULL; x=(x^(x>>30))*0xbf58476d1ce4e5b9ULL; x=(x^(x>>27))*0x94d049bb133111ebULL; return x^(x>>31); }
static inline uint64_t key64(uint32_t c,uint32_t p){return (uint64_t(c)<<32)|uint64_t(p);} 
static inline const char* next_int(const char* p,const char* e,int&out){
  while(p<e && (*p==','||*p=='\n'||*p=='\r'))++p; int v=0; bool neg=false; if(p<e&&*p=='-'){neg=true;++p;} while(p<e&&*p>='0'&&*p<='9'){v=v*10+(*p-'0');++p;} out=neg?-v:v; while(p<e&&*p!=',')++p; return p<e?p+1:e;
}
static void die_sql(sqlite3* db,const char* msg){cerr<<msg<<": "<<sqlite3_errmsg(db)<<"\n"; exit(3);} 
int main(){
 const string raw="/mnt/data/instacart_raw"; const string dbpath="/mnt/data/instacart_project/data/gold/instacart_mart.sqlite";
 ifstream of(raw+"/orders.csv",ios::binary); if(!of){cerr<<"open orders failed\n";return 1;} string line; getline(of,line); vector<OrderInfo> order(3500001); vector<int32_t> custOrders(210001,0); int maxCust=0;
 while(getline(of,line)){
  const char* p=line.data(),*e=p+line.size(); int oid,uid,onum; p=next_int(p,e,oid);p=next_int(p,e,uid); string eval; // parse third field directly
  while(p<e&&*p!=',')++p; // p at comma after eval? not suitable
  // easier parse line positions
  size_t a=line.find(','); size_t b=line.find(',',a+1); size_t c=line.find(',',b+1); size_t d=line.find(',',c+1);
  oid=stoi(line.substr(0,a)); uid=stoi(line.substr(a+1,b-a-1)); string ev=line.substr(b+1,c-b-1); onum=stoi(line.substr(c+1,d-c-1));
  if((size_t)oid>=order.size()) order.resize(oid+1); order[oid]={(int32_t)uid,(int16_t)onum,ev=="prior"}; if(ev=="prior"){ if((size_t)uid>=custOrders.size()) custOrders.resize(uid+1); custOrders[uid]++; maxCust=max(maxCust,uid); }
 }
 of.close(); cerr<<"orders loaded customers="<<maxCust<<"\n";
 size_t cap=1ULL<<24; vector<uint64_t> keys(cap,0); vector<Agg> vals(cap); size_t used=0;
 auto rehash=[&](size_t newcap){vector<uint64_t> nk(newcap,0);vector<Agg>nv(newcap);size_t mask=newcap-1;for(size_t i=0;i<keys.size();++i)if(keys[i]){size_t j=splitmix64(keys[i])&mask;while(nk[j])j=(j+1)&mask;nk[j]=keys[i];nv[j]=vals[i];}keys.swap(nk);vals.swap(nv);cap=newcap;};
 ifstream pf(raw+"/order_products__prior.csv",ios::binary); if(!pf){cerr<<"open prior failed\n";return 1;} getline(pf,line); uint64_t rows=0,unmatched=0;
 while(getline(pf,line)){
  const char* p=line.data(),*e=p+line.size(); int oid,pid,cart,reord; p=next_int(p,e,oid);p=next_int(p,e,pid);p=next_int(p,e,cart);p=next_int(p,e,reord);
  if(oid<=0 || (size_t)oid>=order.size() || !order[oid].valid){unmatched++;continue;} uint32_t cid=order[oid].customer; uint64_t k=key64(cid,pid)+1ULL;
  if((used+1)*100>=cap*72)rehash(cap*2); size_t mask=cap-1,idx=splitmix64(k)&mask;while(keys[idx]&&keys[idx]!=k)idx=(idx+1)&mask;if(!keys[idx]){keys[idx]=k;vals[idx]=Agg{};used++;}
  Agg&a=vals[idx];a.purchase++;a.reorder+=reord;a.first_order=min(a.first_order,order[oid].order_number);a.last_order=max(a.last_order,order[oid].order_number);a.cart_sum+=cart;a.cart_n++;rows++;
 }
 pf.close(); cerr<<"prior rows="<<rows<<" unique_pairs="<<used<<" unmatched="<<unmatched<<"\n";
 remove(dbpath.c_str()); sqlite3*db=nullptr; if(sqlite3_open(dbpath.c_str(),&db)!=SQLITE_OK)die_sql(db,"sqlite open");
 sqlite3_exec(db,"PRAGMA journal_mode=OFF; PRAGMA synchronous=OFF; PRAGMA temp_store=MEMORY; PRAGMA cache_size=-200000;",nullptr,nullptr,nullptr);
 const char* ddl="CREATE TABLE fct_customer_product(customer_id INTEGER NOT NULL,product_id INTEGER NOT NULL,purchase_count INTEGER NOT NULL,reorder_count INTEGER NOT NULL,reorder_rate REAL NOT NULL,first_purchase_order_number INTEGER NOT NULL,last_purchase_order_number INTEGER NOT NULL,purchase_span_orders INTEGER NOT NULL,customer_prior_order_count INTEGER NOT NULL,order_penetration REAL NOT NULL,avg_add_to_cart_order REAL NOT NULL,PRIMARY KEY(customer_id,product_id));";
 if(sqlite3_exec(db,ddl,nullptr,nullptr,nullptr)!=SQLITE_OK)die_sql(db,"ddl");
 sqlite3_stmt* st=nullptr; const char* ins="INSERT INTO fct_customer_product VALUES(?,?,?,?,?,?,?,?,?,?,?)"; if(sqlite3_prepare_v2(db,ins,-1,&st,nullptr)!=SQLITE_OK)die_sql(db,"prepare");
 sqlite3_exec(db,"BEGIN;",nullptr,nullptr,nullptr); size_t n=0; uint64_t sum=0; size_t bad=0;
 for(size_t i=0;i<cap;++i)if(keys[i]){
  uint64_t rk=keys[i]-1ULL; uint32_t cid=rk>>32, pid=rk&0xffffffffU; const Agg&a=vals[i]; int den=custOrders[cid]; double rr=double(a.reorder)/double(a.purchase); double pen=double(a.purchase)/double(den); double avg=double(a.cart_sum)/double(a.cart_n); if(den<=0||rr<0||rr>1||pen<=0||pen>1)bad++;
  sqlite3_bind_int(st,1,(int)cid);sqlite3_bind_int(st,2,(int)pid);sqlite3_bind_int(st,3,a.purchase);sqlite3_bind_int(st,4,a.reorder);sqlite3_bind_double(st,5,rr);sqlite3_bind_int(st,6,a.first_order);sqlite3_bind_int(st,7,a.last_order);sqlite3_bind_int(st,8,a.last_order-a.first_order);sqlite3_bind_int(st,9,den);sqlite3_bind_double(st,10,pen);sqlite3_bind_double(st,11,avg);
  if(sqlite3_step(st)!=SQLITE_DONE)die_sql(db,"insert"); sqlite3_reset(st); sqlite3_clear_bindings(st); n++; sum+=a.purchase; if(n%250000==0){sqlite3_exec(db,"COMMIT; BEGIN;",nullptr,nullptr,nullptr); cerr<<"inserted="<<n<<"\n";}
 }
 sqlite3_exec(db,"COMMIT;",nullptr,nullptr,nullptr); sqlite3_finalize(st);
 cerr<<"insert done n="<<n<<" sum="<<sum<<" bad="<<bad<<"\n";
 sqlite3_exec(db,"CREATE INDEX idx_fcp_product ON fct_customer_product(product_id); CREATE INDEX idx_fcp_customer ON fct_customer_product(customer_id); ANALYZE;",nullptr,nullptr,nullptr);
 sqlite3_close(db);
 ofstream mf("/mnt/data/instacart_project/data/gold/fct_customer_product_validation.csv"); mf<<"metric,value\nsource_prior_rows,"<<rows<<"\nsource_prior_distinct_customer_product_pairs,"<<used<<"\nmart_rows,"<<n<<"\npurchase_count_sum,"<<sum<<"\npurchase_count_reconciles,"<<(sum==rows)<<"\nunmatched_order_product_rows,"<<unmatched<<"\ninvalid_relationship_rows,"<<bad<<"\n";
 cerr<<"DB written "<<dbpath<<"\n"; return (sum==rows&&unmatched==0&&bad==0)?0:2;
}
