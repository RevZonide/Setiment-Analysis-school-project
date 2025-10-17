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

struct WordFreq {
    string word;
    int count;
    
    bool operator<(const WordFreq& other) const {
        return count > other.count;
    }
};

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
        string pos[] = {"suka", "bagus", "baik", "senang", "enak", "praktis", 
                       "mudah", "memudahkan", "canggih", "modern", "seru",
                       "efisien", "cepat", "simple"};
        for(const auto& word : pos) {
            positiveWords.insert(word);
        }
        
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
        
        if(lower.find("tidak") != string::npos && lower.find("suka") != string::npos) {
            return "negative";
        }
        else if(lower.find("iya") != string::npos || 
                (lower.find("suka") != string::npos && lower.find("tidak") == string::npos)) {
            return "positive";
        }
        return "neutral";
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
        fields.push_back(field);
        
        return fields;
    }

public:
    SentimentAnalyzer() {
        initializeStopWords();
        initializeSentimentWords();
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
        
        getline(file, line);
        
        while(getline(file, line)) {
            if(line.empty()) continue;
            lineCount++;
            
            vector<string> fields = parseCSVLine(line);
            
            if(fields.size() >= 4) {
                string sentimentChoice = fields.size() > 3 ? fields[3] : "";
                string reason = fields.size() > 4 ? fields[4] : "";
                
                sentimentChoice.erase(0, sentimentChoice.find_first_not_of(" \t\r\n"));
                sentimentChoice.erase(sentimentChoice.find_last_not_of(" \t\r\n") + 1);
                
                cout << "Line " << lineCount << " sentiment: [" << sentimentChoice << "]" << endl;
                
                string sentiment = analyzeSentimentFromChoice(sentimentChoice);
                if(sentiment == "positive") result.positive++;
                else if(sentiment == "negative") result.negative++;
                else result.neutral++;
                
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
    
    void generatePosterHTML(const map<string, int>& wordFreq, const string& outputFile, const SentimentResult& result, const string& githubURL) {
        vector<WordFreq> words;
        
        for(const auto& pair : wordFreq) {
            words.push_back({pair.first, pair.second});
        }
        
        sort(words.begin(), words.end());
        
        ofstream html(outputFile);
        
        html << "<!DOCTYPE html>\n<html>\n<head>\n";
        html << "<meta charset='UTF-8'>\n";
        html << "<title>Poster - Analisis Survey Checklock</title>\n";
        html << "<style>\n";
        html << "@page { size: A4; margin: 0; }\n";
        html << "* { margin: 0; padding: 0; box-sizing: border-box; }\n";
        html << "body { font-family: 'Segoe UI', Arial, sans-serif; background: white; }\n";
        html << ".poster { width: 210mm; height: 297mm; padding: 15mm; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); position: relative; }\n";
        html << ".content { background: white; height: 100%; border-radius: 15px; padding: 20px; box-shadow: 0 10px 40px rgba(0,0,0,0.3); display: flex; flex-direction: column; }\n";
        html << ".header { text-align: center; margin-bottom: 15px; }\n";
        html << "h1 { color: #667eea; font-size: 32px; margin-bottom: 5px; }\n";
        html << ".subtitle { color: #666; font-size: 14px; }\n";
        html << ".stats-row { display: flex; justify-content: space-around; margin: 15px 0; gap: 10px; }\n";
        html << ".stat-box { flex: 1; text-align: center; padding: 12px; border-radius: 10px; background: #f8f9fa; }\n";
        html << ".stat-number { font-size: 28px; font-weight: bold; color: #667eea; }\n";
        html << ".stat-label { font-size: 12px; color: #666; margin-top: 3px; }\n";
        html << ".chart-section { flex: 1; display: flex; gap: 15px; margin: 10px 0; }\n";
        html << ".bar-chart-container { flex: 0.4; display: flex; flex-direction: column; }\n";
        html << ".chart-title { font-size: 16px; font-weight: bold; color: #333; margin-bottom: 10px; text-align: center; }\n";
        html << ".bar-chart { display: flex; justify-content: space-around; align-items: flex-end; height: 180px; padding: 10px; background: #fafafa; border-radius: 10px; }\n";
        html << ".bar-wrapper { display: flex; flex-direction: column; align-items: center; flex: 1; }\n";
        html << ".bar { width: 50px; border-radius: 6px 6px 0 0; display: flex; align-items: flex-end; justify-content: center; color: white; font-weight: bold; font-size: 16px; padding-bottom: 8px; }\n";
        html << ".positive-bar { background: linear-gradient(to top, #10b981, #34d399); }\n";
        html << ".neutral-bar { background: linear-gradient(to top, #f59e0b, #fbbf24); }\n";
        html << ".negative-bar { background: linear-gradient(to top, #ef4444, #f87171); }\n";
        html << ".bar-label { margin-top: 8px; font-weight: bold; color: #333; font-size: 11px; }\n";
        html << ".bar-count { margin-top: 3px; font-size: 10px; color: #666; }\n";
        html << ".sentiment-detail { display: flex; flex-direction: column; gap: 8px; margin-top: 10px; }\n";
        html << ".sentiment-item { display: flex; align-items: center; gap: 8px; padding: 8px; border-radius: 8px; background: white; }\n";
        html << ".sentiment-icon { width: 30px; height: 30px; border-radius: 50%; display: flex; align-items: center; justify-content: center; font-size: 16px; }\n";
        html << ".positive-icon { background: #d1fae5; }\n";
        html << ".neutral-icon { background: #fef3c7; }\n";
        html << ".negative-icon { background: #fee2e2; }\n";
        html << ".sentiment-text { flex: 1; font-size: 11px; }\n";
        html << ".sentiment-count { font-weight: bold; color: #667eea; font-size: 13px; }\n";
        html << ".wordcloud-container { flex: 0.6; display: flex; flex-direction: column; }\n";
        html << ".word-cloud { display: flex; flex-wrap: wrap; justify-content: center; align-items: center; gap: 6px; padding: 10px; background: #fafafa; border-radius: 10px; flex: 1; overflow: hidden; }\n";
        html << ".word { display: inline-block; padding: 4px 10px; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; border-radius: 5px; font-weight: bold; white-space: nowrap; }\n";
        html << ".footer { display: flex; justify-content: space-between; align-items: center; margin-top: 10px; padding-top: 10px; border-top: 2px solid #e5e7eb; }\n";
        html << ".qr-section { display: flex; align-items: center; gap: 10px; }\n";
        html << ".qr-code { width: 80px; height: 80px; }\n";
        html << ".qr-text { font-size: 10px; color: #666; }\n";
        html << ".footer-text { font-size: 10px; color: #666; text-align: right; }\n";
        html << "@media print { body { margin: 0; } .poster { box-shadow: none; } }\n";
        html << "</style>\n</head>\n<body>\n";
        
        html << "<div class='poster'>\n";
        html << "<div class='content'>\n";
        
        html << "<div class='header'>\n";
        html << "<h1>Analisis Survey Checklock</h1>\n";
        html << "<p class='subtitle'>Hasil Survey Kepuasan Sistem Absensi Checklock</p>\n";
        html << "</div>\n";
        
        int total = result.positive + result.neutral + result.negative;
        html << "<div class='stats-row'>\n";
        html << "<div class='stat-box'>\n";
        html << "<div class='stat-number'>" << total << "</div>\n";
        html << "<div class='stat-label'>Total Responden</div>\n";
        html << "</div>\n";
        html << "<div class='stat-box' style='background: #d1fae5;'>\n";
        html << "<div class='stat-number' style='color: #10b981;'>" << result.positive << "</div>\n";
        html << "<div class='stat-label'>Suka</div>\n";
        html << "</div>\n";
        html << "<div class='stat-box' style='background: #fef3c7;'>\n";
        html << "<div class='stat-number' style='color: #f59e0b;'>" << result.neutral << "</div>\n";
        html << "<div class='stat-label'>Netral</div>\n";
        html << "</div>\n";
        html << "<div class='stat-box' style='background: #fee2e2;'>\n";
        html << "<div class='stat-number' style='color: #ef4444;'>" << result.negative << "</div>\n";
        html << "<div class='stat-label'>Tidak Suka</div>\n";
        html << "</div>\n";
        html << "</div>\n";
        
        html << "<div class='chart-section'>\n";
        
        html << "<div class='bar-chart-container'>\n";
        html << "<div class='chart-title'>Hasil Sentimen</div>\n";
        html << "<div class='bar-chart'>\n";
        
        int maxCount = max({result.positive, result.neutral, result.negative});
        
        int positiveHeight = maxCount > 0 ? (result.positive * 150 / maxCount) : 0;
        html << "<div class='bar-wrapper'>\n";
        html << "<div class='bar positive-bar' style='height: " << positiveHeight << "px;'>" << result.positive << "</div>\n";
        html << "<div class='bar-label'>Suka</div>\n";
        html << "</div>\n";
        
        int neutralHeight = maxCount > 0 ? (result.neutral * 150 / maxCount) : 0;
        html << "<div class='bar-wrapper'>\n";
        html << "<div class='bar neutral-bar' style='height: " << neutralHeight << "px;'>" << result.neutral << "</div>\n";
        html << "<div class='bar-label'>Netral</div>\n";
        html << "</div>\n";
        
        int negativeHeight = maxCount > 0 ? (result.negative * 150 / maxCount) : 0;
        html << "<div class='bar-wrapper'>\n";
        html << "<div class='bar negative-bar' style='height: " << negativeHeight << "px;'>" << result.negative << "</div>\n";
        html << "<div class='bar-label'>Tidak</div>\n";
        html << "</div>\n";
        
        html << "</div>\n";
        
        html << "<div class='sentiment-detail'>\n";
        html << "<div class='sentiment-item'>\n";
        html << "<div class='sentiment-icon positive-icon'>+</div>\n";
        html << "<div class='sentiment-text'>" << result.positive << " responden menjawab <b>iya suka!</b></div>\n";
        html << "</div>\n";
        html << "<div class='sentiment-item'>\n";
        html << "<div class='sentiment-icon neutral-icon'>-</div>\n";
        html << "<div class='sentiment-text'>" << result.neutral << " responden menjawab <b>netral</b></div>\n";
        html << "</div>\n";
        html << "<div class='sentiment-item'>\n";
        html << "<div class='sentiment-icon negative-icon'>x</div>\n";
        html << "<div class='sentiment-text'>" << result.negative << " responden menjawab <b>tidak suka!</b></div>\n";
        html << "</div>\n";
        html << "</div>\n";
        
        html << "</div>\n";
        
        html << "<div class='wordcloud-container'>\n";
        html << "<div class='chart-title'>Kata yang Sering Muncul</div>\n";
        html << "<div class='word-cloud'>\n";
        
        for(int i = 0; i < min(25, (int)words.size()); i++) {
            int fontSize = 10 + (words[i].count * 2);
            fontSize = min(fontSize, 28);
            html << "<span class='word' style='font-size: " << fontSize << "px;'>" 
                 << words[i].word << " (" << words[i].count << ")</span>\n";
        }
        
        html << "</div>\n";
        html << "</div>\n";
        
        html << "</div>\n";
        
        html << "<div class='footer'>\n";
        html << "<div class='qr-section'>\n";
        html << "<img class='qr-code' src='https://api.qrserver.com/v1/create-qr-code/?size=200x200&data=" << githubURL << "' alt='QR Code'>\n";
        html << "<div class='qr-text'><b>Scan untuk kode sumber</b><br>GitHub Repository</div>\n";
        html << "</div>\n";
        html << "<div class='footer-text'>\n";
        html << "Dibuat dengan C++ Sentiment Analysis<br>\n";
        html << "Data dianalisis dari " << total << " responden survey\n";
        html << "</div>\n";
        html << "</div>\n";
        
        html << "</div>\n";
        html << "</div>\n";
        
        html << "</body>\n</html>";
        html.close();
        
        cout << "Poster HTML generated: " << outputFile << endl;
    }
};

int main() {
    cout << "=== Sentiment Analysis & Word Cloud Generator ===" << endl;
    cout << "Converting Excel/CSV data to word cloud...\n" << endl;
    
    SentimentAnalyzer analyzer;
    
    string filename = "survey_data.csv";
    cout << "Reading file: " << filename << endl;
    
    SentimentResult result = analyzer.analyzeCSV(filename);
    
    analyzer.displaySentimentStats(result);
    
    analyzer.generateWordCloud(result.wordFrequency, 20);
    
    string githubURL = "https://github.com/yourusername/sentiment-analysis";
    cout << "\nEnter your GitHub repository URL (or press Enter to use default): ";
    string userGithubURL;
    getline(cin, userGithubURL);
    if(!userGithubURL.empty()) {
        githubURL = userGithubURL;
    }
    
    analyzer.generatePosterHTML(result.wordFrequency, "poster.html", result, githubURL);
    
    cout << "\n=== FILES GENERATED ===" << endl;
    cout << "poster.html - A4 size poster (open and print to PDF or save as image)" << endl;
    cout << "\nTo convert to PNG/JPG:" << endl;
    cout << "1. Open poster.html in your browser" << endl;
    cout << "2. Press Ctrl+P (or Cmd+P on Mac)" << endl;
    cout << "3. Choose 'Save as PDF' or use browser screenshot tools" << endl;
    cout << "4. Or use online tools to convert the PDF to PNG/JPG" << endl;
    
    return 0;
}