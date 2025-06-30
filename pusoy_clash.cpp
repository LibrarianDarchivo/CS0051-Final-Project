#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <cstdlib>
#include <chrono>
#include <condition_variable>

using namespace std;

const int NUM_PLAYERS = 4;
const int NUM_ROUNDS = 3;

mutex mtx;
condition_variable cv;
int playsThisRound = 0;
int playersReady = 0;
bool isRoundComplete = false;

vector<int> drawnCards(NUM_PLAYERS, 0);
vector<int> playerScores(NUM_PLAYERS, 0);

void drawCard(int playerId) {
    srand(static_cast<unsigned int>(time(0)) + playerId);

    for (int round = 0; round < NUM_ROUNDS; ++round)
    {
        this_thread::sleep_for(chrono::milliseconds(300 + playerId * 150));

        int card = (rand() % 13) + 2;
        {
            lock_guard<mutex> lock(mtx);
            drawnCards[playerId] = card;
            cout << "Player " << playerId << " draws a card: " << card << endl;
            playsThisRound++;
        }

        cv.notify_all();
        {
            unique_lock<mutex> lock(mtx);
            cv.wait(lock, [] { return isRoundComplete; });
        }

        {
            lock_guard<mutex> lock(mtx);
            playersReady++;
        }

        cv.notify_all();
        {
            unique_lock<mutex> lock(mtx);
            cv.wait(lock, [] { return !isRoundComplete; });
        }
    }
}

void determineRoundWinner(int round) {
    int highestCard = -1;
    int winnerId = -1;

    for (int i = 0; i < NUM_PLAYERS; ++i) {
        if (drawnCards[i] > highestCard) {
            highestCard = drawnCards[i];
            winnerId = i;
        }
    }

    playerScores[winnerId]++;
    cout << "\nRound " << round + 1 << " winner: Player " << winnerId
         << " with card " << highestCard << "!\n";
}

int main() {
    cout << "=== Pusoy Clash Game Simulation ===\n";

    vector<thread> players;
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        players.emplace_back(drawCard, i);
    }

    for (int round = 0; round < NUM_ROUNDS; ++round) {
        {
            unique_lock<mutex> lock(mtx);
            cout << "\n--- Starting Round " << round + 1 << " ---\n";
        }

        {
            unique_lock<mutex> lock(mtx);
            cv.wait(lock, [] { return playsThisRound == NUM_PLAYERS; });
        }

        determineRoundWinner(round);

        {
            lock_guard<mutex> lock(mtx);
            playsThisRound = 0;
            isRoundComplete = true;
        }
        cv.notify_all();

        {
            unique_lock<mutex> lock(mtx);
            cv.wait(lock, [] { return playersReady == NUM_PLAYERS; });
            playersReady = 0;
            isRoundComplete = false;
        }
        cv.notify_all();
    }

    for (auto& t : players) t.join();

    cout << "\n=== Final Scores ===\n";
    int maxScore = -1;
    vector<int> winners;

    for (int i = 0; i < NUM_PLAYERS; ++i) {
        cout << "Player " << i << ": " << playerScores[i] << " points\n";
        if (playerScores[i] > maxScore) {
            maxScore = playerScores[i];
            winners.clear();
            winners.push_back(i);
        } else if (playerScores[i] == maxScore) {
            winners.push_back(i);
        }
    }

    if (winners.size() == 1) {
        cout << "\n🏆Player " << winners[0] << " won with " << maxScore << " points!\n";
    } else {
        cout << "\nDraw between players: ";
        for (size_t i = 0; i < winners.size(); ++i) {
            cout << winners[i];
            if (i < winners.size() - 1) cout << ", ";
        }
        cout << " with " << maxScore << " point(s) each!\n";
    }

    return 0;
}
