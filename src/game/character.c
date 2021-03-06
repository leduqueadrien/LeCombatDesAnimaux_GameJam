
#include <SANDAL2/SANDAL2.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include "../structure.h"
#include "fight.h"
#include "character.h"
#include "animations.h"
#include "movements.h"
#include "attacks.h"


void keyOnActions(Element * character, SDL_Keycode k) {
	DataCharacter_t * d = character->data;
	KeyCodes_t * kc = d->keyCodes;

	/*Si on appuie sur une touche d'attaque*/
	if (k == kc->attack1 || k == kc->attack2 || k == kc->parry) {
		launchCharacterAttack(character, k);
	}

	/*Si on appuie sur une touche de déplacement*/
	else if (k == kc->left || k == kc->right || k == kc->jump) {
		moveCharacterOn(character, k);
	}

}


void keyOffActions(Element * character, SDL_Keycode k) {
	DataCharacter_t * d = character->data;
	KeyCodes_t * kc = d->keyCodes;

	/*Si on relache une touche de déplacement*/
	if (k == kc->left || k == kc->right || k == kc->jump) {
		moveCharacterOff(character, k);
	}
}


void actionCharacters(Element * character) {
	moveCharacter(character);
	jumpCharacter(character);
	collisionCharacters(character);
	charactersAnimation(character);
}


void initCharacter(int idPlayer, int idChara, Element ** character) {
	DataCharacter_t * d = initDataCharacter(idPlayer, idChara);
	double x = 0;
	double y = 0;
	char   filename[50];
	double groundLevel = HFEN-50-(d->height);

	if (!getCharactersFilename(idChara, filename)) {
		printf("Error fetching character's filename\n");
	} else {

		if (d != NULL) {

			if (idPlayer == PLAYER_L) {
				x = 100;
				y = groundLevel;
			} else { // PLAYER_R
				x = LFEN-100-(d->width);
				y = groundLevel;
			}

			(*character) = createImage(x, y, d->width, d->height, filename, FIGHT_SCREEN, PlanCharacters);
			createCharacterAnimations(*character);

			if (idPlayer == PLAYER_R) {
				(*character)->flip = SANDAL2_FLIP_HOR;
			}

			(*character)->data = d;
		}
	}
}


DataCharacter_t * initDataCharacter(int idPlayer, int idChosen) {
	DataCharacter_t * d = (DataCharacter_t *)malloc(sizeof(DataCharacter_t));
	KeyCodes_t * kc = (KeyCodes_t *)malloc(sizeof(KeyCodes_t));
	HealthBar_t * hb = (HealthBar_t *)malloc(sizeof(HealthBar_t));

	if (d != NULL && kc != NULL && hb != NULL) {

		initKeyCodes(kc, idPlayer);
		d->keyCodes = kc;

		d->winNum = 0;
		initHealthBar(hb, idPlayer);
		d->healthBar = hb;

		FILE * file = NULL;
		file = fopen("assets/stats/DataCharacters.txt", "r");

		if (file != NULL) {

			int idChara = -1;
			char line[6];

			d->idPlayer = idPlayer;

			d->left       = false;
			d->allowLeft  = false;
			d->right      = false;
			d->allowRight = false;
			d->jump       = false;
			d->allowJump  = true;
			d->crouch     = false;

			d->attacks      = false;
			d->allowAttacks = true;

			d->attack1      = false;
			d->attack2      = false;
			d->parry        = false;


			while (!feof(file) && idChara != idChosen) {
				fgets(line, 6, file);
				idChara = atoi(line);
			}
			d->idChara = idChara;

			fgets(line, 6, file);
			d->hpCte = atoi(line);
			d->hp = d->hpCte;

			fgets(line, 6, file);
			d->strengthCte = atoi(line);

			fgets(line, 6, file);
			d->speedCte = atoi(line);
			d->speed = d->speedCte;

			fgets(line, 6, file);
			d->width = atoi(line);

			fgets(line, 6, file);
			d->height = atoi(line);

			fgets(line, 6, file);
			d->jumpForceCte = atoi(line);
			d->jumpForceTmp = d->jumpForceCte;

			fgets(line, 6, file);
			d->jumpLagCte = atoi(line);
		}

		fclose(file);
	}

	return d;
}


void initKeyCodes(KeyCodes_t * kc, int idPlayer) {

	if (idPlayer == PLAYER_L) {
		kc->left    = 'a';
		kc->right   = 'e';
		kc->jump    = 'z';
		kc->attack1 = 's';
		kc->attack2 = 'f';
		kc->parry   = 'd';

	} else { //idPlayer == PLAYER_R
		kc->left    = 'y';
		kc->right   = 'i';
		kc->jump    = 'u';
		kc->attack1 = 'j';
		kc->attack2 = 'l';
		kc->parry   = 'k';
	}
}


void initHealthBar(HealthBar_t * hb, int idPlayer) {
	int white[4] = {255, 255, 255, 255};
	int red[4] = {255, 0, 0, 255};
	int orange[4] = {255, 100, 0, 255};
	int blue[4] = {0, 0, 255, 255};

	int marge = 0.05 * LFEN;
	int yBar = marge/2;
	int xBarRight = LFEN / 2 + marge;
	int wBar = 0.4 * LFEN;
	int hBar = 0.05 * HFEN;

	int sideBubble = 0.8*hBar;
	int yBubble = yBar+hBar+1;

	Element * healthBar = NULL;
	Element * bubble1 = NULL;
	Element * bubble2 = NULL;

	if (idPlayer == PLAYER_L) { //Left
		createBlock(marge, yBar, wBar, hBar, white, FIGHT_SCREEN, PlanHealthBars); // Contour blanc du block Left
		healthBar = createBlock(marge+2, yBar+2, wBar-4, hBar-4, red, FIGHT_SCREEN, PlanHealthBars-1); // contenu rouge du block Left
		createBlock(marge-hBar, yBar, hBar, hBar, orange, FIGHT_SCREEN, PlanHealthBars-1); //Logo Perso Left

		bubble1 = createBlock(marge+wBar-2.5*sideBubble, yBubble, sideBubble, sideBubble, blue, FIGHT_SCREEN, PlanHealthBars-1); //Bubble1Left du BO3
		bubble2 = createBlock(marge+wBar-sideBubble, yBubble, sideBubble, sideBubble, blue, FIGHT_SCREEN, PlanHealthBars-1); //Bubble2Left du BO3

	} else { //Right
		createBlock(xBarRight, yBar, wBar, hBar, white, FIGHT_SCREEN, PlanHealthBars); // Contour blanc du block Right
		healthBar = createBlock(xBarRight+2, yBar+2, wBar-4, hBar-4, red, FIGHT_SCREEN, PlanHealthBars-1); // contenu rouge du block Right
		createBlock(xBarRight+wBar, yBar, hBar, hBar, orange, FIGHT_SCREEN, PlanHealthBars-1); //Logo Perso Right

		bubble1 = createBlock(xBarRight, yBubble, sideBubble, sideBubble, blue, FIGHT_SCREEN, PlanHealthBars-1); //Bubble1Right du BO3
		bubble2 = createBlock(xBarRight+1.5*sideBubble, yBubble, sideBubble, sideBubble, blue, FIGHT_SCREEN, PlanHealthBars-1); //Bubble2Right du BO3
	}

	if (healthBar != NULL && bubble1 != NULL && bubble2 != NULL) {
		hb->healthBar = healthBar;
		hb->bubble1 = bubble1;
		hb->bubble2 = bubble2;
	}
}


int getCharactersFilename(int idChara, char filename[50]) {
	int error = 1;
	DIR * rep = opendir("./assets/sprites");
	char filenameTmp[30] = "s_.png";

	if (rep != NULL) {
		struct dirent * ent = NULL;

		while ((ent = readdir(rep)) != NULL) {
			if (ent->d_name[1] - 48 == idChara) {
				strcpy(filenameTmp, ent->d_name);
			}
		}

		if (filenameTmp[1] == '_') {
			error = 0;
		}

		snprintf(filename, 50, "./assets/sprites/%s", filenameTmp);

		closedir(rep);
	} else {
		error = 0;
	}
	return error;
}

