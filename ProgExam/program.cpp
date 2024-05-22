#include <iostream>
#include <fstream>

using namespace std;

int main()
{
    int pgsize, cache_size, fifo = 0, lru = 0, opt = 0;
    ifstream inp_file;
    inp_file.open("inp.txt");
    if(!inp_file.is_open()){
        cout << "ERROR: Input file missing.";
        exit(0);
    }
    inp_file >> cache_size >> pgsize; //reading physical frames and page size from input file.
    int n = 0;
    int temp[10000];
    while(!inp_file.eof()){
        inp_file >> temp[n]; //reading the addresses being accessed
        n++;
    }
    int page_access_order[n];
    for(int i=0; i<n; i++) page_access_order[i] = temp[i]/pgsize; // converting addresses to page numbers.
    inp_file.close();

    int fifo_cache[cache_size], lru_cache[cache_size], opt_cache[cache_size]; //creating the caches for fifo, lru and opt.
    for(int i = 0; i<cache_size; i++) {fifo_cache[i] = -1; lru_cache[i] = -1; opt_cache[i] = -1;} //clearing the caches
    int pages_in_cache = 0;
    for(int i=0; i<n; i++){ //for each page being accessed
        bool fifo_in_cache = false, lru_in_cache = false, opt_in_cache = false;
        int found_at = 0;
        for(int j = 0; j < pages_in_cache; j++){ //searching for the page in each cache
            if(fifo_cache[j] == page_access_order[i]) fifo_in_cache = true;
            if(opt_cache[j] == page_access_order[i]) opt_in_cache = true;
            if(lru_cache[j] == page_access_order[i]){
                lru_in_cache = true;
                found_at = j; //for lru, we also need the index in cache where element is found
            }
        }
        if(lru_in_cache){ //in lru, if page is in cache, move it to end of cache
            for(int j=found_at; j<pages_in_cache-1; j++) lru_cache[j] = lru_cache[j+1];
            lru_cache[pages_in_cache-1] = page_access_order[i];
        }
        if(!fifo_in_cache){
            fifo++;
            if(pages_in_cache < cache_size){ //if cache has space, all replacement algorithms will fault. Thus add page to each cache
                fifo_cache[pages_in_cache] = page_access_order[i];
                lru_cache[pages_in_cache] = page_access_order[i];
                opt_cache[pages_in_cache] = page_access_order[i];
                lru_in_cache = true;
                opt_in_cache = true;
                lru++;
                opt++;
                pages_in_cache++;
            } else{ //if cache doesn't have space, pop first page in cache and push new page
                for(int j=0; j<pages_in_cache-1; j++) fifo_cache[j] = fifo_cache[j+1];
                fifo_cache[pages_in_cache-1] = page_access_order[i];
            }
        }
        if(!lru_in_cache){ //if cache doesn't have space, pop first page in cache and push new page
            lru++;
            for(int j=0; j<pages_in_cache-1; j++) lru_cache[j] = lru_cache[j+1];
            lru_cache[pages_in_cache-1] = page_access_order[i];
        }
        if(!opt_in_cache){
            opt++;
            int last = 0;
            int target = 0;
            for(int j=0; j<pages_in_cache; j++){ //find the page not being used for maximum time
                int next_access = -1;
                for(int k = i+1; k < n; k++){
                    if(page_access_order[k] == opt_cache[j]){
                        next_access = k;
                        break;
                    }
                }
                if(next_access > last){
                    last = next_access;
                    target = j;
                }
                if(next_access == -1){
                    target = j;
                    break;
                }
            }
            opt_cache[target] = page_access_order[i]; //replace the page not being used for maximum time
        }
    }
    cout << "Number of page faults with FIFO: " << fifo << "\n"; //print page faults
    cout << "Number of page faults with LRU: " << lru << "\n";
    cout << "Number of page faults with OPT: " << opt << "\n";
    return 0;
}
