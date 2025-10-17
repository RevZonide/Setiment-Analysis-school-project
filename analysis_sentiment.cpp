#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <cctype>
#include <set>

using namespace std;

// Structure to hold word frequency
struct WordFreq {
    string word;
    int count;
    
    bool operator<(const WordFreq& other) const {
        return count > other.count; // Sort descending
    }
};

// Structure for sentiment analysis
struct SentimentResult {
    int positive = 0;
    int negative = 0;
    int neutral = 0;
    map<string, int> wordFrequency;
};

class SentimentAnalyzer {
private:
    set<string> stopWords;
    set<string> positiveWords;
    set<string> negativeWords;
    
    void initializeStopWords() {
        // Indonesian stop words
        string stops[] = {"yang", "di", "ke", "dari", "ini", "itu", "untuk", 
                         "dan", "atau", "dengan", "pada", "adalah", "ada", 
                         "saya", "aku", "kamu", "dia", "kita", "mereka",
                         "jika", "kalau", "kalo", "tapi", "tetapi", "namun",
                         "karena", "karna", "kalo", "gak", "ga", "tidak",
                         "sih", "aja", "aj"};
        for(const auto& word : stops) {
            stopWords.insert(word);
        }
    }
    
    void initializeSentimentWords() {
        // Positive words
        string pos[] = {"suka", "bagus", "baik", "senang", "enak", "praktis", 
                       "mudah", "memudahkan", "canggih", "modern", "seru",
                       "efisien", "cepat", "simple"};
        for(const auto& word : pos) {
            positiveWords.insert(word);
        }
        
        // Negative words
        string neg[] = {"tidak", "ribet", "ruwet", "susah", "lama", "malas",
                       "males", "error", "lag", "repot", "lambat", "buruk",
                       "jelek", "bosan", "antri", "ngantri", "menghambat"};
        for(const auto& word : neg) {
            negativeWords.insert(word);
        }
    }
    
    string toLowerCase(string str) {
        transform(str.begin(), str.end(), str.begin(), ::tolower);
        return str;
    }
    
    string cleanWord(string word) {
        string cleaned;
        for(char c : word) {
            if(isalnum(c)) {
                cleaned += tolower(c);
            }
        }
        return cleaned;
    }
    
    string analyzeSentimentFromChoice(const string& choice) {
        string lower = toLowerCase(choice);
        
        // Check for negative first (more specific)
        if(lower.find("tidak") != string::npos && lower.find("suka") != string::npos) {
            return "negative";
        }
        // Then check for positive
        else if(lower.find("iya") != string::npos || 
                (lower.find("suka") != string::npos && lower.find("tidak") == string::npos)) {
            return "positive";
        }
        return "neutral";
    }

public:
    SentimentAnalyzer() {
        initializeStopWords();
        initializeSentimentWords();
    }
    
    vector<string> parseCSVLine(const string& line) {
        vector<string> fields;
        string field;
        bool inQuotes = false;
        
        for(size_t i = 0; i < line.length(); i++) {
            char c = line[i];
            
            if(c == '"') {
                inQuotes = !inQuotes;
            } else if(c == ',' && !inQuotes) {
                fields.push_back(field);
                field.clear();
            } else {
                field += c;
            }
        }
        fields.push_back(field); // Add last field
        
        return fields;
    }
    
    SentimentResult analyzeCSV(const string& filename) {
        SentimentResult result;
        ifstream file(filename);
        string line;
        int lineCount = 0;
        
        if(!file.is_open()) {
            cerr << "Error: Could not open file " << filename << endl;
            return result;
        }
        
        // Skip header
        getline(file, line);
        
        while(getline(file, line)) {
            if(line.empty()) continue;
            lineCount++;
            
            vector<string> fields = parseCSVLine(line);
            
            // Debug: Print what we're parsing
            if(fields.size() >= 4) {
                string sentimentChoice = fields.size() > 3 ? fields[3] : "";
                string reason = fields.size() > 4 ? fields[4] : "";
                
                // Clean up the sentiment choice (remove extra spaces)
                sentimentChoice.erase(0, sentimentChoice.find_first_not_of(" \t\r\n"));
                sentimentChoice.erase(sentimentChoice.find_last_not_of(" \t\r\n") + 1);
                
                cout << "Line " << lineCount << " sentiment: [" << sentimentChoice << "]" << endl;
                
                // Analyze sentiment from choice
                string sentiment = analyzeSentimentFromChoice(sentimentChoice);
                if(sentiment == "positive") result.positive++;
                else if(sentiment == "negative") result.negative++;
                else result.neutral++;
                
                // Process reason for word cloud
                processText(reason, result.wordFrequency);
            }
        }
        
        file.close();
        cout << "\nProcessed " << lineCount << " responses." << endl;
        return result;
    }
    
    void processText(const string& text, map<string, int>& wordFreq) {
        stringstream ss(text);
        string word;
        
        while(ss >> word) {
            word = cleanWord(word);
            if(word.length() > 2 && stopWords.find(word) == stopWords.end()) {
                wordFreq[word]++;
            }
        }
    }
    
    void displaySentimentStats(const SentimentResult& result) {
        int total = result.positive + result.negative + result.neutral;
        
        cout << "\n========== SENTIMENT ANALYSIS RESULTS ==========" << endl;
        cout << "Total Responses: " << total << endl;
        cout << "Positive (Suka): " << result.positive 
             << " (" << (total > 0 ? (result.positive * 100.0 / total) : 0) << "%)" << endl;
        cout << "Negative (Tidak Suka): " << result.negative 
             << " (" << (total > 0 ? (result.negative * 100.0 / total) : 0) << "%)" << endl;
        cout << "Neutral: " << result.neutral 
             << " (" << (total > 0 ? (result.neutral * 100.0 / total) : 0) << "%)" << endl;
        cout << "================================================\n" << endl;
    }
    
    void generateWordCloud(const map<string, int>& wordFreq, int topN = 20) {
        vector<WordFreq> words;
        
        for(const auto& pair : wordFreq) {
            words.push_back({pair.first, pair.second});
        }
        
        sort(words.begin(), words.end());
        
        cout << "\n========== WORD CLOUD (Top " << min(topN, (int)words.size()) << " Words) ==========" << endl;
        
        for(int i = 0; i < min(topN, (int)words.size()); i++) {
            string bar(words[i].count * 2, '#');
            cout << words[i].word << " (" << words[i].count << "): " << bar << endl;
        }
        
        cout << "================================================\n" << endl;
    }
    
    void generateHTMLWordCloud(const map<string, int>& wordFreq, const string& outputFile, const SentimentResult& result) {
        vector<WordFreq> words;
        
        for(const auto& pair : wordFreq) {
            words.push_back({pair.first, pair.second});
        }
        
        sort(words.begin(), words.end());
        
        ofstream html(outputFile);
        
        html << "<!DOCTYPE html>\n<html>\n<head>\n";
        html << "<meta charset='UTF-8'>\n";
        html << "<title>Word Cloud - Checklock Survey</title>\n";
        html << "<style>\n";
        html << "body { font-family: Arial, sans-serif; background: #f0f0f0; padding: 20px; }\n";
        html << ".container { max-width: 1200px; margin: 0 auto; background: white; padding: 30px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }\n";
        html << "h1 { text-align: center; color: #333; }\n";
        html << "h2 { text-align: center; color: #555; margin-top: 40px; }\n";
        html << ".chart-container { margin: 40px auto; max-width: 600px; }\n";
        html << ".bar-chart { display: flex; justify-content: space-around; align-items: flex-end; height: 300px; padding: 20px; background: #fafafa; border-radius: 10px; margin-bottom: 10px; }\n";
        html << ".bar-wrapper { display: flex; flex-direction: column; align-items: center; flex: 1; margin: 0 10px; }\n";
        html << ".bar { width: 80px; border-radius: 8px 8px 0 0; transition: transform 0.3s; display: flex; align-items: flex-end; justify-content: center; color: white; font-weight: bold; font-size: 20px; padding-bottom: 10px; }\n";
        html << ".bar:hover { transform: translateY(-5px); }\n";
        html << ".positive-bar { background: linear-gradient(to top, #10b981, #34d399); }\n";
        html << ".neutral-bar { background: linear-gradient(to top, #f59e0b, #fbbf24); }\n";
        html << ".negative-bar { background: linear-gradient(to top, #ef4444, #f87171); }\n";
        html << ".bar-label { margin-top: 10px; font-weight: bold; color: #333; }\n";
        html << ".bar-count { margin-top: 5px; font-size: 14px; color: #666; }\n";
        html << ".word-cloud { display: flex; flex-wrap: wrap; justify-content: center; gap: 10px; padding: 20px; }\n";
        html << ".word { display: inline-block; padding: 5px 15px; margin: 5px; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; border-radius: 5px; font-weight: bold; transition: transform 0.2s; }\n";
        html << ".word:hover { transform: scale(1.1); }\n";
        html << "</style>\n</head>\n<body>\n";
        html << "<div class='container'>\n";
        html << "<h1>📊 Analisis Survey Checklock</h1>\n";
        
        // Bar Chart Section
        html << "<h2>Hasil Sentimen</h2>\n";
        html << "<div class='chart-container'>\n";
        html << "<div class='bar-chart'>\n";
        
        int total = result.positive + result.neutral + result.negative;
        int maxCount = max({result.positive, result.neutral, result.negative});
        
        // Positive Bar
        int positiveHeight = maxCount > 0 ? (result.positive * 250 / maxCount) : 0;
        html << "<div class='bar-wrapper'>\n";
        html << "<div class='bar positive-bar' style='height: " << positiveHeight << "px;'>" << result.positive << "</div>\n";
        html << "<div class='bar-label'>✅ Suka</div>\n";
        html << "<div class='bar-count'>" << result.positive << " menjawab iya suka!</div>\n";
        html << "</div>\n";
        
        // Neutral Bar
        int neutralHeight = maxCount > 0 ? (result.neutral * 250 / maxCount) : 0;
        html << "<div class='bar-wrapper'>\n";
        html << "<div class='bar neutral-bar' style='height: " << neutralHeight << "px;'>" << result.neutral << "</div>\n";
        html << "<div class='bar-label'>😐 Netral</div>\n";
        html << "<div class='bar-count'>" << result.neutral << " menjawab netral!</div>\n";
        html << "</div>\n";
        
        // Negative Bar
        int negativeHeight = maxCount > 0 ? (result.negative * 250 / maxCount) : 0;
        html << "<div class='bar-wrapper'>\n";
        html << "<div class='bar negative-bar' style='height: " << negativeHeight << "px;'>" << result.negative << "</div>\n";
        html << "<div class='bar-label'>❌ Tidak Suka</div>\n";
        html << "<div class='bar-count'>" << result.negative << " menjawab tidak suka!</div>\n";
        html << "</div>\n";
        
        html << "</div>\n";
        html << "</div>\n";
        
        // Word Cloud Section
        html << "<h2>Word Cloud - Kata yang Sering Muncul</h2>\n";
        html << "<div class='word-cloud'>\n";
        
        for(int i = 0; i < min(30, (int)words.size()); i++) {
            int fontSize = 12 + (words[i].count * 3);
            fontSize = min(fontSize, 48);
            html << "<span class='word' style='font-size: " << fontSize << "px;'>" 
                 << words[i].word << " (" << words[i].count << ")</span>\n";
        }
        
        html << "</div>\n</div>\n</body>\n</html>";
        html.close();
        
        cout << "HTML word cloud generated: " << outputFile << endl;
    }
};

int main() {
    cout << "=== Sentiment Analysis & Word Cloud Generator ===" << endl;
    cout << "Converting Excel/CSV data to word cloud...\n" << endl;
    
    SentimentAnalyzer analyzer;
    
    // Analyze the CSV file
    string filename = "survey_data.csv";
    cout << "Reading file: " << filename << endl;
    
    SentimentResult result = analyzer.analyzeCSV(filename);
    
    // Display sentiment statistics
    analyzer.displaySentimentStats(result);
    
    // Generate text-based word cloud
    analyzer.generateWordCloud(result.wordFrequency, 20);
    
    // Generate HTML word cloud
    analyzer.generateHTMLWordCloud(result.wordFrequency, "wordcloud.html", result);
    
    cout << "\nAnalysis complete! Open 'wordcloud.html' in your browser to see the visual word cloud." << endl;
    
    return 0;
}