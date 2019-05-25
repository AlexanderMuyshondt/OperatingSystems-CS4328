# CS4328 PROJECT 2 - Card Matching Fight
In this project, we are going to build “Card Matching Fight”. Card matching fight is a simple card game with one dealer,
3 players, and a single deck of cards. The game is composed of three rounds. At the beginning of each round, the dealer
shuffles the cards, deals the first three cards and waits for the round to finish, before repeating the same process for
the next round. A winner in a round is the first player to get a match (i.e., two cards of equal rank). In each round, a
different player is given the privilege to start.
Initially, the dealer shuffles the deck of cards and hands each player a single card in a round robin fashion (say starting
from player 1 in round 1). Once the dealer is done handling the cards, the dealer places the remaining deck of cards on the
table and the first round begins. Each player (starting from player 1), draws a card from the deck and compares it to the 
card he/she has. If they match, the player shows the hand, declares him/herself as a winner and the round completes.
Otherwise, the player will discard one card (at random) by placing it at the end of the deck of cards on the table and the
next player proceeds. Once a round ends, the dealer will shuffle the deck and hands a card to each player. In the second
round, the second player starts drawing a card from the deck. In the third round, the third player starts drawing a card
from the deck.

## Downloading
Simply clone the GitHub repository and all the code is ready to go. No extra requirements to install.

`$ git clone https://github.com/AlexanderMuyshondt/cardMatchingFight`

## How to run
1. From terminal, navigate into project folder.
2. Run make.
3. Type ./CardMatchingFight integer [integer being a number for the random seed].
	- EXAMPLE: ./CardMatchingFight 2500
