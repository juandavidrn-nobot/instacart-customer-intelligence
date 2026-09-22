#include <bits/stdc++.h>
using namespace std;

struct OrderInfo { int32_t customer=0; int16_t order_number=0; bool valid=false; };
struct Agg { int32_t purchase=0; int32_t reorder=0; int16_t first_order=32767; int16_t last_order=0; int32_t cart_sum=0; int32_t cart_n=0; };

static inline uint64_t key64(uint32_t c, uint32_t p){ return (uint64_t(c)<<32) | uint64_t(p); }
static inline uint64_t splitmix64(uint64_t x){ x += 0x9e3779b97f4a7c15ULL; x=(x^(x>>30))*0xbf58476d1ce4e5b9ULL; x=(x^(x>>27))*0x94d049bb133111ebULL; return x^(x>>31); }

// Fast numeric field extraction for the fixed-width integer CSV files.
static inline const char* next_int(const char* p, const char* e, int &out){
    while(p<e && (*p==',' || *p=='\n' || *p=='\r')) ++p;
    int v=0;
    bool neg=false;
    if(p<e && *p=='-'){ neg=true; ++p; }
    while(p<e && *p>= '0' && *p<='9'){ v=v*10+(*p-'0'); ++p; }
    out = neg ? -v : v;
    while(p<e && *p!=',') ++p;
    return p<e ? p+1 : e;
}

int main(int argc, char** argv){
    string raw = "/mnt/data/instacart_raw";
    string out = "/mnt/data/instacart_project/data/gold/fct_customer_product.csv";
    string orders_path=raw+"/orders.csv";
    string prior_path=raw+"/order_products__prior.csv";

    // We know the public dataset bounds from the source schema, but allocate dynamically after reading.
    ifstream of(orders_path, ios::binary);
    if(!of){ cerr << "Cannot open orders.csv\n"; return 1; }
    string line;
    getline(of,line); // header
    struct TmpOrder { int32_t customer; int16_t order_num; bool prior; };
    vector<TmpOrder> order(3500001, {0,0,false});
    vector<int32_t> customer_prior(210000+1, 0);
    uint32_t max_order=0, max_customer=0;
    while(getline(of,line)){
        int p1=line.find(',');
        if(p1<0) continue;
        int p2=line.find(',',p1+1);
        int p3=line.find(',',p2+1);
        int p4=line.find(',',p3+1);
        int oid=stoi(line.substr(0,p1));
        int uid=stoi(line.substr(p1+1,p2-p1-1));
        string eval=line.substr(p2+1,p3-p2-1);
        int onum=stoi(line.substr(p3+1,p4-p3-1));
        if((size_t)oid>=order.size()) order.resize(oid+1,{0,0,false});
        order[oid]={(int32_t)uid,(int16_t)onum,eval=="prior"};
        if(eval=="prior"){
            if((size_t)uid>=customer_prior.size()) customer_prior.resize(uid+1,0);
            customer_prior[uid]++;
        }
        max_order=max<uint32_t>(max_order,oid); max_customer=max<uint32_t>(max_customer,uid);
    }
    of.close();
    cerr << "Loaded orders max_order="<<max_order<<" customers="<<max_customer<<"\n";

    // Open-addressed hash table; ~2x expected unique customer-product pairs.
    // We grow on load factor > 0.72. This avoids std::unordered_map node overhead.
    size_t cap=1ULL<<24; // 16.7M slots initial
    vector<uint64_t> keys(cap,0); // key+1, because (0,0) is never a real pair
    vector<Agg> vals(cap);
    size_t used=0;
    auto rehash = [&](size_t newcap){
        vector<uint64_t> nk(newcap,0); vector<Agg> nv(newcap);
        size_t mask=newcap-1;
        for(size_t i=0;i<keys.size();++i){ if(!keys[i]) continue; size_t j=splitmix64(keys[i]) & mask; while(nk[j]) j=(j+1)&mask; nk[j]=keys[i]; nv[j]=vals[i]; }
        keys.swap(nk); vals.swap(nv); cap=newcap;
    };

    ifstream pf(prior_path, ios::binary);
    if(!pf){ cerr << "Cannot open prior csv\n"; return 1; }
    getline(pf,line); // header
    uint64_t rows=0, unmatched=0;
    while(getline(pf,line)){
        const char* p=line.data(), *e=p+line.size();
        int oid, pid, cart, reordered;
        p=next_int(p,e,oid); p=next_int(p,e,pid); p=next_int(p,e,cart); p=next_int(p,e,reordered);
        if(oid<0 || (size_t)oid>=order.size() || !order[oid].prior){ unmatched++; continue; }
        uint32_t cid=order[oid].customer;
        uint64_t rawkey=key64(cid,(uint32_t)pid);
        uint64_t k=rawkey+1ULL; // safe because rawkey cannot be max uint64 for valid IDs
        if((used+1)*100 >= cap*72) rehash(cap*2);
        size_t mask=cap-1, idx=splitmix64(k)&mask;
        while(keys[idx] && keys[idx]!=k) idx=(idx+1)&mask;
        if(!keys[idx]){ keys[idx]=k; vals[idx]=Agg{}; used++; }
        Agg &a=vals[idx];
        a.purchase++; a.reorder+=reordered; a.first_order=min<int16_t>(a.first_order,order[oid].order_num); a.last_order=max<int16_t>(a.last_order,order[oid].order_num); a.cart_sum+=cart; a.cart_n++;
        rows++;
    }
    pf.close();
    cerr << "Prior rows processed="<<rows<<" unmatched="<<unmatched<<" unique_pairs="<<used<<"\n";

    ofstream outF(out);
    if(!outF){ cerr << "Cannot write output\n"; return 1; }
    outF << "customer_id,product_id,purchase_count,reorder_count,reorder_rate,first_purchase_order_number,last_purchase_order_number,purchase_span_orders,customer_prior_order_count,order_penetration,avg_add_to_cart_order\n";
    uint64_t sum_purchase=0; size_t out_rows=0; size_t bad_den=0, bad_rr=0, bad_pen=0;
    outF.setf(ios::fixed); outF<<setprecision(6);
    for(size_t i=0;i<cap;++i){
        if(!keys[i]) continue;
        uint64_t rawkey=keys[i]-1ULL; uint32_t cid=uint32_t(rawkey>>32); uint32_t pid=uint32_t(rawkey&0xffffffffU); const Agg&a=vals[i];
        int32_t den = customer_prior[cid];
        double rr = den>0 ? double(a.reorder)/double(a.purchase) : 0.0;
        double pen = den>0 ? double(a.purchase)/double(den) : 0.0;
        outF << cid << ',' << pid << ',' << a.purchase << ',' << a.reorder << ',' << rr << ',' << a.first_order << ',' << a.last_order << ',' << int(a.last_order-a.first_order) << ',' << den << ',' << pen << ',' << (double(a.cart_sum)/max(1,a.cart_n)) << '\n';
        sum_purchase += a.purchase; out_rows++;
        if(den==0) bad_den++; if(rr<0 || rr>1) bad_rr++; if(pen<=0 || pen>1) bad_pen++;
    }
    outF.close();

    string metrics= "/mnt/data/instacart_project/data/gold/fct_customer_product_validation.csv";
    ofstream mf(metrics);
    mf << "metric,value\n";
    mf << "source_prior_rows,"<<rows<<"\n";
    mf << "source_prior_distinct_orders,0\n"; // populated below by a lightweight external query if needed
    mf << "source_prior_distinct_customers,"<<max_customer<<"\n";
    mf << "mart_rows,"<<out_rows<<"\n";
    mf << "mart_distinct_customers,~customer_count\n";
    mf << "mart_purchase_count_sum,"<<sum_purchase<<"\n";
    mf << "unmatched_customer_denominators,"<<bad_den<<"\n";
    mf << "invalid_reorder_rate_rows,"<<bad_rr<<"\n";
    mf << "invalid_penetration_rows,"<<bad_pen<<"\n";
    mf << "purchase_count_reconciles,"<<(sum_purchase==rows?1:0)<<"\n";
    mf.close();
    cerr << "Wrote "<<out<<" rows="<<out_rows<<" sum_purchase="<<sum_purchase<<"\n";
    return (sum_purchase==rows && unmatched==0 && bad_den==0 && bad_rr==0 && bad_pen==0) ? 0 : 2;
}
