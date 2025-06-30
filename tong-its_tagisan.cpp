#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <random>
#include <chrono>
#include <condition_variable>

using namespace std;

const int NUM_PLAYERS = 3;
const int NUM_ROUNDS = 5;

mutex mtx;
condition_variable cv;
int playsThisRound = 0;
int playersReadyForNext = 0;
bool roundComplete = false;

vector<int> roundCards(NUM_PLAYERS, 0);
vector<int> scores(NUM_PLAYERS, 0);

void player(int id) {
    random_device rd;
    mt19937 rng(rd());
    uniform_int_distribution<int> dist(1, 13);

    for (int round = 0; round < NUM_ROUNDS; ++round) {
        this_thread::sleep_for(chrono::milliseconds(300 + id * 150));

        int card = dist(rng);
        {
            lock_guard<mutex> lock(mtx);
            roundCards[id] = card;
            cout << "[Player " << id << "] discards " << card << endl;
            playsThisRound++;
        }

        cv.notify_one();

        {
            unique_lock<mutex> lock(mtx);
            cv.wait(lock, [] { return roundComplete; });
        }

        {
            lock_guard<mutex> lock(mtx);
            playersReadyForNext++;
        }

        cv.notify_one();

        {
            unique_lock<mutex> lock(mtx);
            cv.wait(lock, [] { return !roundComplete; });
        }
    }
}

int main() {
    cout << "=== Tong-its Tagisan Simulation (5 Rounds) ===\n";

    vector<thread> players;
    for (int i = 0; i < NUM_PLAYERS; ++i)
        players.emplace_back(player, i);

    for (int round = 0; round < NUM_ROUNDS; ++round) {
        {
            unique_lock<mutex> lock(mtx);
            cout << "\n------------------------\n";
            cout << " Starting Round " << round + 1 << "\n";
            cout << "------------------------\n";
        }

        {
            unique_lock<mutex> lock(mtx);
            cv.wait(lock, [] { return playsThisRound == NUM_PLAYERS; });
        }

        int maxVal = -1, winner = -1;
        for (int i = 0; i < NUM_PLAYERS; ++i) {
            if (roundCards[i] > maxVal) {
                maxVal = roundCards[i];
                winner = i;
            }
        }

        scores[winner]++;
        cout << "\n[Round " << round + 1 << "] Winner: Player " << winner
             << " (Card: " << maxVal << ")\n";

        {
            lock_guard<mutex> lock(mtx);
            playsThisRound = 0;
            roundComplete = true;
        }
        cv.notify_all();

        {
            unique_lock<mutex> lock(mtx);
            cv.wait(lock, [] { return playersReadyForNext == NUM_PLAYERS; });
            playersReadyForNext = 0;
            roundComplete = false;
        }
        cv.notify_all();
    }

    for (auto& t : players) t.join();

    cout << "\n=== Final Scores ===\n";
    int maxScore = -1;
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        cout << "Player " << i << ": " << scores[i] << " points\n";
        if (scores[i] > maxScore) maxScore = scores[i];
    }

    // Determine winner(s)
    vector<int> winners;
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        if (scores[i] == maxScore) {
            winners.push_back(i);
        }
    }

    if (winners.size() == 1) {
        cout << "\n🏆 Player " << winners[0] << " is the overall winner with "
             << maxScore << " points!\n";
    } else {
        cout << "\n🤝 It's a tie between ";
        for (size_t i = 0; i < winners.size(); ++i) {
            cout << "Player " << winners[i];
            if (i < winners.size() - 1)
                cout << ", ";
        }
        cout << " with " << maxScore << " points each!\n";
    }

    return 0;
}
