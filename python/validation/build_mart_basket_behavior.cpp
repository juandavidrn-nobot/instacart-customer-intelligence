#include <bits/stdc++.h>
#include <sqlite3.h>
using namespace std;
struct OrderInfo { int32_t customer=0; int16_t order_number=0; int8_t dow=0, hour=0; int8_t days_valid=0; int8_t days=0; bool prior=false; };
struct ProductInfo { int16_t aisle=0; int8_t dept=0; };
static inline const char* next_int(const char* p,const char* e,int&out){
  while(p<e && (*p==','||*p=='\n'||*p=='\r'))++p; int v=0; bool neg=false; if(p<e&&*p=='-'){neg=true;++p;} while(p<e&&*p>='0'&&*p<='9'){v=v*10+(*p-'0');++p;} out=neg?-v:v; while(p<e&&*p!=',')++p; return p<e?p+1:e;
}
static void die(sqlite3* db,const char* m){cerr<<m<<": "<<sqlite3_errmsg(db)<<"\n"; exit(3);} 
static inline int popcount64(uint64_t x){return __builtin_popcountll(x);} 
int main(){
 const string raw="/mnt/data/instacart_raw"; const string dbp="/mnt/data/instacart_project/data/gold/instacart_mart.sqlite";
 vector<ProductInfo> prod(50000);
 ifstream pf(raw+"/products.csv"); if(!pf){cerr<<"products open fail\n";return 1;} string line; getline(pf,line); while(getline(pf,line)){
   size_t a=line.find(','); size_t c=line.rfind(','); size_t b=line.rfind(',',c-1); int pid=stoi(line.substr(0,a)); int aisle=stoi(line.substr(b+1,c-b-1)); int dept=stoi(line.substr(c+1)); if(pid>=(int)prod.size())prod.resize(pid+1); prod[pid]={(int16_t)aisle,(int8_t)dept}; }
 pf.close();
 vector<OrderInfo> ord(3500001);
 ifstream of(raw+"/orders.csv",ios::binary); if(!of){cerr<<"orders open fail\n";return 1;} getline(of,line); int priorOrders=0;
 while(getline(of,line)){
   size_t a=line.find(','),b=line.find(',',a+1),c=line.find(',',b+1),d=line.find(',',c+1),e=line.find(',',d+1);
   int oid=stoi(line.substr(0,a)); int uid=stoi(line.substr(a+1,b-a-1)); string ev=line.substr(b+1,c-b-1); int onum=stoi(line.substr(c+1,d-c-1)); int dow=stoi(line.substr(d+1,e-d-1)); string tail=line.substr(e+1); size_t comma=tail.find(','); int hour=stoi(tail.substr(0,comma)); string ds=tail.substr(comma+1);
   int days=0; bool dv=!ds.empty(); if(dv) days=stoi(ds);
   if((size_t)oid>=ord.size())ord.resize(oid+1); ord[oid]={(int32_t)uid,(int16_t)onum,(int8_t)dow,(int8_t)hour,(int8_t)dv,(int8_t)days,ev=="prior"}; if(ev=="prior")priorOrders++;
 }
 of.close(); cerr<<"loaded prior orders="<<priorOrders<<"\n";
 sqlite3*db=nullptr; if(sqlite3_open(dbp.c_str(),&db)!=SQLITE_OK)die(db,"sqlite open");
 sqlite3_exec(db,"PRAGMA journal_mode=OFF; PRAGMA synchronous=OFF; PRAGMA temp_store=MEMORY; PRAGMA cache_size=-200000;",nullptr,nullptr,nullptr);
 sqlite3_exec(db,"DROP TABLE IF EXISTS mart_basket_behavior;",nullptr,nullptr,nullptr);
 const char *ddl="CREATE TABLE mart_basket_behavior(order_id INTEGER PRIMARY KEY,customer_id INTEGER NOT NULL,order_number INTEGER NOT NULL,order_dow INTEGER NOT NULL,order_hour_of_day INTEGER NOT NULL,days_since_prior_order INTEGER,first_order_flag INTEGER NOT NULL,basket_size INTEGER NOT NULL,unique_aisles INTEGER NOT NULL,unique_departments INTEGER NOT NULL,reordered_items INTEGER NOT NULL,new_to_customer_items INTEGER NOT NULL,reorder_share REAL NOT NULL,new_to_customer_share REAL NOT NULL,avg_add_to_cart_order REAL NOT NULL,max_add_to_cart_order INTEGER NOT NULL);";
 if(sqlite3_exec(db,ddl,nullptr,nullptr,nullptr)!=SQLITE_OK)die(db,"ddl");
 sqlite3_stmt* st=nullptr; const char *ins="INSERT INTO mart_basket_behavior VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)"; if(sqlite3_prepare_v2(db,ins,-1,&st,nullptr)!=SQLITE_OK)die(db,"prepare");
 ifstream f(raw+"/order_products__prior.csv",ios::binary); if(!f){cerr<<"prior open fail\n";return 1;} getline(f,line);
 int current=-1; int basket=0,reord=0; long long cartsum=0; int cartmax=0; uint64_t aisleLo=0,aisleHi=0; uint32_t deptMask=0; long long rows=0; int ordersWritten=0; long long sumBasket=0;
 auto flush=[&](){ if(current<0)return; const auto&o=ord[current]; int ua=popcount64(aisleLo)+popcount64(aisleHi); int ud=__builtin_popcount(deptMask); int newi=basket-reord; double rr=double(reord)/basket, nr=double(newi)/basket, avg=double(cartsum)/basket;
   sqlite3_bind_int(st,1,current);sqlite3_bind_int(st,2,o.customer);sqlite3_bind_int(st,3,o.order_number);sqlite3_bind_int(st,4,o.dow);sqlite3_bind_int(st,5,o.hour); if(o.days_valid)sqlite3_bind_int(st,6,o.days); else sqlite3_bind_null(st,6);sqlite3_bind_int(st,7,o.order_number==1);sqlite3_bind_int(st,8,basket);sqlite3_bind_int(st,9,ua);sqlite3_bind_int(st,10,ud);sqlite3_bind_int(st,11,reord);sqlite3_bind_int(st,12,newi);sqlite3_bind_double(st,13,rr);sqlite3_bind_double(st,14,nr);sqlite3_bind_double(st,15,avg);sqlite3_bind_int(st,16,cartmax); if(sqlite3_step(st)!=SQLITE_DONE)die(db,"insert"); sqlite3_reset(st);ordersWritten++;sumBasket+=basket;
 };
 sqlite3_exec(db,"BEGIN;",nullptr,nullptr,nullptr);
 while(getline(f,line)){
   const char*p=line.data(),*e=line.data()+line.size(); int oid,pid,cart,reorder; p=next_int(p,e,oid);p=next_int(p,e,pid);p=next_int(p,e,cart);p=next_int(p,e,reorder); rows++;
   if(oid!=current){ if(current!=-1)flush(); current=oid; basket=0;reord=0;cartsum=0;cartmax=0;aisleLo=aisleHi=0;deptMask=0; }
   basket++; reord+=reorder; cartsum+=cart; cartmax=max(cartmax,cart); auto pi=prod[pid]; if(pi.aisle>0){int x=pi.aisle-1; if(x<64)aisleLo|=(1ULL<<x); else aisleHi|=(1ULL<<(x-64));} if(pi.dept>0)deptMask|=(1u<<(pi.dept-1));
 }
 if(current!=-1)flush(); sqlite3_exec(db,"COMMIT;",nullptr,nullptr,nullptr); sqlite3_finalize(st); f.close();
 sqlite3_exec(db,"CREATE INDEX idx_basket_customer_order ON mart_basket_behavior(customer_id,order_number); CREATE INDEX idx_basket_dow_hour ON mart_basket_behavior(order_dow,order_hour_of_day); ANALYZE;",nullptr,nullptr,nullptr);
 ofstream vf("/mnt/data/instacart_project/data/gold/mart_basket_behavior_validation.csv"); vf<<"metric,value\nsource_prior_item_rows,"<<rows<<"\nmart_order_rows,"<<ordersWritten<<"\nsum_basket_size,"<<sumBasket<<"\nbasket_size_reconciles,"<<(sumBasket==rows)<<"\nfirst_order_rows,";
 sqlite3_stmt*q=nullptr; sqlite3_prepare_v2(db,"SELECT COUNT(*) FROM mart_basket_behavior WHERE first_order_flag=1",-1,&q,nullptr); sqlite3_step(q); vf<<sqlite3_column_int64(q,0)<<"\nsource_prior_order_count,"<<priorOrders<<"\norder_count_reconciles,"<<(ordersWritten==priorOrders)<<"\nreorder_share_out_of_range,"; sqlite3_finalize(q); sqlite3_prepare_v2(db,"SELECT COUNT(*) FROM mart_basket_behavior WHERE reorder_share<0 OR reorder_share>1 OR new_to_customer_share<0 OR new_to_customer_share>1",-1,&q,nullptr); sqlite3_step(q); vf<<sqlite3_column_int64(q,0)<<"\n"; sqlite3_finalize(q);
 sqlite3_close(db); cerr<<"basket mart complete orders="<<ordersWritten<<" item_rows="<<rows<<"\n"; return 0;
}
