CXX = g++
CXXFLAGS = -std=c++11 -O2
LDFLAGS = -lcurl -pthread

WHITELIST_CSV = https://raw.githubusercontent.com/ercexpo/us-news-domains/refs/heads/main/us-news-domains-v2.0.0.csv
WHITELIST_TXT = src/whitelist.txt

all: $(WHITELIST_TXT) bin/12ftpp

$(WHITELIST_TXT):
	@echo "Fetching CSV and extracting domains..."
	curl -fsSL "$(WHITELIST_CSV)" \
	  | tail -n +2 \
	  | cut -d, -f1 \
	  | sed '/^\s*$$/d' \
	  > "$(WHITELIST_TXT)"
	@echo "Generated $(WHITELIST_TXT) with $$(wc -l < $(WHITELIST_TXT)) entries"

bin/12ftpp: src/main.cpp $(WHITELIST_TXT)
	mkdir -p bin
	$(CXX) $(CXXFLAGS) src/main.cpp -o bin/12ftpp $(LDFLAGS)

clean:
	rm -rf bin $(WHITELIST_TXT)
