#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <ctime>

class Card {
public:
	enum rank { ACE = 1, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN, JACK, QUEEN, KING };
	enum suit { CLUBS, DIAMONDS, HEARTS, SPADES };

	//перегружаю оператор << чтобы можно было отправить объект типа Card
	//в стандартный поток вывода
	friend std::ostream& operator<<(std::ostream& os, const Card& aCard);
	Card(rank r = ACE, suit s = SPADES, bool ifu = true);

	//возвращает значение карты от 1 до 11
	int getValue() const;

	//переворачевает карту
	void Flip();
private:
	rank m_Rank;
	suit m_Suit;
	bool m_isFaceUp;
};

Card::Card(rank r, suit s, bool ifu) : m_Rank(r), m_Suit(s), m_isFaceUp(ifu) {

}

int Card::getValue() const {
	int value = 0; //если карта лицом вниз, то ее значение = 0

	if (m_isFaceUp) {
		value = m_Rank;
		if (value > 10) {
			value = 10;
		}
	}
	return value;
}

void Card::Flip() {
	m_isFaceUp = !(m_isFaceUp);
}

class Hand {
public:
	Hand();
	virtual ~Hand();
	void Add(Card* pCard); //добавлаем карту в руку
	void Clear(); //убираем карты из рук

	//зачисление очков(1-11) в зависимости от карт
	int getTotal() const;
protected:
	std::vector<Card*> m_Cards;
};

Hand::Hand() {
	m_Cards.reserve(7);
}

Hand::~Hand() {
	Clear();
}

void Hand::Add(Card* pCard) {
	m_Cards.push_back(pCard);
}

void Hand::Clear() {
	//проходим по вектору освобождая всю память в куче
	std::vector<Card*>::iterator iter = m_Cards.begin();

	for (iter = m_Cards.begin(); iter != m_Cards.end(); ++iter) {
		delete* iter;
		*iter = nullptr;
	}
	//очищаем вектор указателей
	m_Cards.clear();
}

int Hand::getTotal() const {
	if (m_Cards.empty()) { //если в руке нет карт, то возвращаем 0
		return 0;
	}

	//если у первой карты значение = 0, то она лежит рубашкой вверх
	//возвращаем 0
	if (m_Cards[0]->getValue() == 0) {
		return 0;
	}

	int total = 0;
	std::vector<Card*>::const_iterator iter;

	for (iter = m_Cards.begin(); iter != m_Cards.end(); ++iter) {
		total += (*iter)->getValue();
	}

	bool containAce = false; //смотрим есть ли в руке туз
	for (iter = m_Cards.begin(); iter != m_Cards.end(); ++iter) {
		if ((*iter)->getValue() == Card::ACE) {
			containAce = true;
		}
	}

	//если в руке есть туз и сума очков <= 11, то туз дает +11 очков
	if (containAce && total <= 11) {
		total += 10; //тут даем 10 очков ибо мы уже дали за каждый туз +1
	}
	return total;
}

class GenericPlayer : public Hand {
	friend std::ostream& operator<<(std::ostream& os, const GenericPlayer& aGenericPlayer);
public:
	GenericPlayer(const std::string& name = " ");
	virtual ~GenericPlayer();
	virtual bool isHitting() const = 0; //показывает хочет ли игрок продолжить игру
	bool isBusted() const; //возвращаем значение если у игрока перебор(очков > 21)
	void Bust() const; //сообщем, что у игрока перебор
protected:
	std::string m_Name;
};

GenericPlayer::GenericPlayer(const std::string& name) : m_Name(name) {

}

GenericPlayer::~GenericPlayer() {

}

bool GenericPlayer::isBusted() const {
	return (getTotal() > 21);
}

void GenericPlayer::Bust() const {
	std::cout << m_Name << " busts.\n";
}

class Player : public GenericPlayer {
public:
	Player(const std::string& name = " ");
	virtual ~Player();
	virtual bool isHitting() const; //хочет ли игрок взять карты
	void Win() const; //объявляем, что игрок победил
	void Lose() const; //объявляем, что игрок проиграл
	void Push() const; //объявляем ничью
};

Player::Player(const std::string& name) : GenericPlayer(name) {

}

Player::~Player() {

}

bool Player::isHitting() const {
	std::cout << m_Name << ", do you want a hit? (y/n): ";
	char response;
	std::cin >> response;

	return(response == 'y' || response == 'Y');
}

void Player::Win() const {
	std::cout << m_Name << " wins.\n";
}

void Player::Lose() const {
	std::cout << m_Name << " loses.\n";
}

void Player::Push() const {
	std::cout << m_Name << " pushes.\n";
}

class House : public GenericPlayer{
public:
	House (const std::string& name = "House");
	virtual ~House ();
	virtual bool isHitting() const;
	void flipFirstCard();
};

House ::House (const std::string& name) : GenericPlayer(name) {

}

House ::~House () {

}

bool House::isHitting() const {
	return (getTotal() <= 16);
}

void House::flipFirstCard() {
	if (!(m_Cards.empty())) {
		m_Cards[0]->Flip();
	}
	else {
		std::cout << "No cards to flip.\n";
	}
}

class Deck : public Hand {
public:
	Deck();
	virtual ~Deck();
	void Populate(); //создаем колоду из 52 карт
	void Shuffle(); //тусуем 
	void Deal(Hand& aHand); //раздаем по 1 карте
	void AdditionalCards(GenericPlayer& aGenericPlayer); //раздаем доп. карты
};

Deck::Deck() {
	m_Cards.reserve(52);
	Populate();
}

Deck::~Deck() {

}

void Deck::Populate() {
	Clear();
	for (int s = Card::CLUBS; s <= Card::SPADES; ++s) {
		for (int r = Card::ACE; r <= Card::KING; ++r) {
			Add(new Card(static_cast<Card::rank>(r), static_cast<Card::suit>(s)));
		}
	}
}

void Deck::Shuffle() {
	random_shuffle(m_Cards.begin(), m_Cards.end());
}

void Deck::Deal(Hand& aHand) {
	if (!m_Cards.empty()) {
		aHand.Add(m_Cards.back());
		m_Cards.pop_back();
	}
	else {
		std::cout << "Out of cards. Unable to deal.";
	}
}

void Deck::AdditionalCards(GenericPlayer& aGenericPlayer) {
	std::cout << std::endl;
	while (!(aGenericPlayer.isBusted()) && aGenericPlayer.isHitting()) {
		Deal(aGenericPlayer);
		std::cout << aGenericPlayer << std::endl;
		if (aGenericPlayer.isBusted()) {
			aGenericPlayer.Bust();
		}
	}
}

class Game {
public:
	Game(const std::vector<std::string>& names);
	~Game();
	void Play();
private:
	Deck m_Deck;
	House m_House;
	std::vector<Player> m_Players;
};

Game::Game(const std::vector<std::string>& names) {
	std::vector<std::string>::const_iterator pName;//создаем вектор игроков
	for (pName = names.begin(); pName != names.end(); ++pName) {
		m_Players.push_back(Player(*pName));
	}
	srand(static_cast<unsigned int>(time(0)));
	m_Deck.Populate();
	m_Deck.Shuffle();
}

Game::~Game() {

}

void Game::Play() {
	std::vector<Player>::iterator pPlayer;

	for (int i = 0; i < 2; ++i) {
		for (pPlayer = m_Players.begin(); pPlayer != m_Players.end(); ++pPlayer) {
			m_Deck.Deal(*pPlayer);
		}
		m_Deck.Deal(m_House);
	}
	m_House.flipFirstCard(); //прячем карту диллера

	for (pPlayer = m_Players.begin(); pPlayer != m_Players.end(); ++pPlayer) {
		m_Deck.AdditionalCards(*pPlayer); //открываем все карты
	}
	m_House.flipFirstCard();
	std::cout << std::endl << m_House; //показяем 1ю карту диллера

	m_Deck.AdditionalCards(m_House);//даем диллеру доп.карты
	if (m_House.isBusted()) {
		for (pPlayer = m_Players.begin(); pPlayer != m_Players.end(); ++pPlayer) {
			if (!(pPlayer->isBusted())) {
				pPlayer->Win();
			}
		}
	}
	else {//сравниваем очки всех оставшихся игроков
		for (pPlayer = m_Players.begin(); pPlayer != m_Players.end(); ++pPlayer) {
			if (!(pPlayer->isBusted())) {
				if (pPlayer->getTotal() > m_House.getTotal()) {
					pPlayer->Win();
				}
				else if (pPlayer->getTotal() < m_House.getTotal()) {
					pPlayer->Lose();
				}
				else {
					pPlayer->Push();
				}
			}
		}
	}
	//убираем карты из рук всех игроков
	for (pPlayer = m_Players.begin(); pPlayer != m_Players.end(); ++pPlayer) {
		pPlayer->Clear();
	}
	m_House.Clear();
}

std::ostream& operator<<(std::ostream& os, const Card& aCard);
std::ostream& operator<<(std::ostream& os, const GenericPlayer& aGenericPlayers);

int main() {
    std::cout << "Welcome to Blackjack!\n\n";

	int numPlayers = 0;

	while (numPlayers < 1 || numPlayers > 7) {
		std::cout << "How many players? (1-7): ";
		std::cin >> numPlayers;
	}
	std::vector<std::string> names;
	std::string name;

	for (int i = 0; i < numPlayers; ++i) {
		std::cout << "Enter player's name: ";
		std::cin >> name;
		names.push_back(name);
	}
	std::cout << std::endl;

	Game aGame(names);
	char again = 'y';

	while (again != 'n' || again != 'N') {
		aGame.Play();
		std::cout << "Do you want to play again? (y/n): ";
		std::cin >> again;
	}

}

//перегружаем оператор << чтобы получить возможность отпрвить объект типа Card 
//в поток cout
std::ostream& operator<<(std::ostream& os, const Card& aCard) {
	const std::string RANKS[]{"0", "A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"};
	const std::string SUITS[]{"c", "d", "h", "s"};

	if (aCard.m_isFaceUp) {
		os << RANKS[aCard.m_Rank] << SUITS[aCard.m_Suit];
	}
	else {
		os << "XX";
	}
	return os;
}


//перегружаем оператор << чтобы получить возможность отпрвить объект типа GenericPlayer 
//в поток cout
std::ostream& operator<<(std::ostream& os, const GenericPlayer& aGenericPlayers) {
	os << aGenericPlayers.m_Name << ":\t";
	std::vector<Card*>::const_iterator pCard;

	if (!aGenericPlayers.m_Cards.empty()) {
		for (pCard = aGenericPlayers.m_Cards.begin(); pCard != aGenericPlayers.m_Cards.end(); ++pCard) {
			os << *(*pCard) << "\t";
		}
		if (aGenericPlayers.getTotal() != 0) {
			std::cout << "(" << aGenericPlayers.getTotal() << ")";
		}
	}
	else {
		os << "<empty>";
	}
	return os;
}