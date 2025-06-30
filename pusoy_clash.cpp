#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
using namespace std;

struct RoundResult {
    int winner;
    int highestCard;
    vector<int> cards;
};

RoundResult playRound() {
    RoundResult result;
    result.cards.resize(4);

    // Each player draws a card (2-14)
    for (int i = 0; i < 4; i++) {
        result.cards[i] = 2 + rand() % 13;
    }

    // Find winner (highest card)
    auto max_it = max_element(result.cards.begin(), result.cards.end());
    result.highestCard = *max_it;
    result.winner = distance(result.cards.begin(), max_it);

    return result;
}

void printCard(int card) {
    if (card <= 10) cout << card;
    else if (card == 11) cout << "J";
    else if (card == 12) cout << "Q";
    else if (card == 13) cout << "K";
    else cout << "A";
}

int main() {
    srand(time(0)); // Seed random number generator

    cout << "Pusoy Clash - 4 Players, 3 Rounds\n";
    cout << "Cards: 2-10, J(11), Q(12), K(13), A(14)\n\n";

    vector<int> wins(4, 0);

    for (int round = 1; round <= 3; round++) {
        cout << "=== Round " << round << " ===\n";
        RoundResult result = playRound();

        // Show cards
        for (int i = 0; i < 4; i++) {
            cout << "Player " << (i+1) << ": ";
            printCard(result.cards[i]);
            cout << endl;
        }

        // Show winner
        cout << "Winner: Player " << (result.winner + 1)
             << " with ";
        printCard(result.highestCard);
        cout << endl << endl;

        wins[result.winner]++;
    }

    // Show final results
    cout << "=== Final Results ===\n";
    for (int i = 0; i < 4; i++) {
        cout << "Player " << (i+1) << " won " << wins[i] << " round(s)\n";
    }

    return 0;
}
