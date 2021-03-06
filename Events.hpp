/*
Cette classe permet de g�rer les evenements a la souris et au clavier.
Elle fait l'interface entre la boucle d'evenements et les methodes de calcul et d'affichage des fractales.
*/

#pragma once

#include <stdio.h>
#include <iostream>
#include <sstream>
#include <chrono>
#include <SDL2/SDL.h>

#include "Parametres.hpp"
#include "Mandel.hpp"
#include "Affichage.hpp"
#include "BigFloat.hpp"
#include "BigMandel.hpp"

using namespace std;


int affichageGPU(Affichage* disp);
int computeBigMandelGPU(Affichage* disp);
int computeBigMandelGPU(Affichage* disp, bool h_posCx, uint32_t* h_decCx, bool h_posCy, uint32_t* h_decCy, uint32_t* h_decS);
void computeMandel(Uint32* result, Complexe& center, float scale);
void computeBigMandel(Uint32* result, BigFloat& xCenter, BigFloat& yCenter, BigFloat& scale);

class Events{
public:
	static BigFloat* bigScale;
	static BigFloat* xCenter;
	static BigFloat* yCenter;
	
	static void initialDisplay(Affichage* disp) {
		bigScale = new BigFloat(disp->scale);

		//Nouveau calcul de la fractale avec chrono
		disp->start = chrono::system_clock::now();

		if (GPU && BIG_FLOAT_SIZE == 0)
		{
			affichageGPU(disp);	
			disp->dessin();
		}
		else if (GPU)
		{
			Events::yCenter = new BigFloat();
			Events::xCenter = new BigFloat();
			//computeBigMandelGPU(disp);
			computeBigMandelGPU(disp, xCenter->pos, xCenter->decimals, yCenter->pos, yCenter->decimals, bigScale->decimals);
		}
		else
			if (BIG_FLOAT_SIZE == 0)
				computeMandel(disp->pixels, disp->center, disp->scale);
			else {
				if (INTERACTIVE) {
					BigFloat xCenter(0);
					BigFloat yCenter(0);
				}
				else {
					Events::yCenter = new BigFloat(true, 0, 121012162, 3888660452, 0);
					Events::xCenter = new BigFloat(false, 1, 3178543730, 764955228, 0);
				}
				computeBigMandel(disp->pixels, *xCenter, *yCenter, *bigScale);
			}

			disp->end = chrono::system_clock::now();
			disp->duration = disp->end - disp->start;
			cout << "Frame computing time : " << disp->duration.count() << endl;

			// Affichage de la fractale
			disp->dessin();
	}

	static void updateBigCenter(SDL_Event& event, bool zoom) {
		BigFloat temp, temp2; 
		BigFloat::mult(((float)event.motion.x) / WIDTH - 0.5f, *bigScale, temp2);
		BigFloat::add(*xCenter, temp2, temp);
		temp2.reset();
		if (zoom)
			BigFloat::mult((1.f - ZOOM_FACTOR), temp, temp2);
		else
			BigFloat::mult((1.f - 1 / DEZOOM_FACTOR), temp, temp2);
		temp.reset();
		if (zoom)
			BigFloat::mult(ZOOM_FACTOR, *xCenter, temp);
		else
			BigFloat::mult(1 / DEZOOM_FACTOR, *xCenter, temp);
		BigFloat::add(temp2, temp, *xCenter);

		temp.reset();
		temp2.reset();
		BigFloat::mult(((float)event.motion.y) / HEIGHT - 0.5f, *bigScale, temp2);
		BigFloat::add(*yCenter, temp2, temp);
		temp2.reset();
		if (zoom)
			BigFloat::mult((1.f - ZOOM_FACTOR), temp, temp2);
		else
			BigFloat::mult((1.f - 1 / DEZOOM_FACTOR), temp, temp2);
		temp.reset();
		if (zoom)
			BigFloat::mult(ZOOM_FACTOR, *yCenter, temp);
		else
			BigFloat::mult(1 / DEZOOM_FACTOR, *yCenter, temp);
		BigFloat::add(temp2, temp, *yCenter);
	}


	// Zoom vers la cible du clic
	static void clicGauche(SDL_Event& event, Affichage* disp)
	{

		// Calcul de la position du clic dans le plan complexe
		float x = disp->center.x + disp->scale*(((float)event.motion.x) / WIDTH - 0.5f);
		float y = disp->center.y + disp->scale*(((float)event.motion.y) / HEIGHT - 0.5f);

		// MAJ de la position du nouveau centre dans le plan complexe
		if (INTERACTIVE) {
			disp->center.x = x + (disp->center.x - x) * ZOOM_FACTOR;
			disp->center.y = y + (disp->center.y - y) * ZOOM_FACTOR;
			updateBigCenter(event, true);
		}
		

		// MAJ de l'echelle
		disp->scale *= ZOOM_FACTOR;
		//BigFloat zoomFactor(true, 0, 0x80000000, 0, 0);
		BigFloat temp, temp2;
		BigFloat::mult(ZOOM_FACTOR, *bigScale, temp);
		bigScale->reset();
		temp2.copy(*bigScale);
		BigFloat::add(temp, temp2, *bigScale);
		
		//Nouveau calcul de la fractale avec chrono
		disp->start = chrono::system_clock::now();

		if (GPU && BIG_FLOAT_SIZE == 0)
			affichageGPU(disp);
		else if (GPU)
			computeBigMandelGPU(disp, xCenter->pos, xCenter->decimals, yCenter->pos, yCenter->decimals, bigScale->decimals);
		else
			if (BIG_FLOAT_SIZE == 0)
				computeMandel(disp->pixels, disp->center, disp->scale);
			else {
				computeBigMandel(disp->pixels, *xCenter, *yCenter, *bigScale);
			}

		disp->end = chrono::system_clock::now();
		disp->duration = disp->end - disp->start;
		cout << "Frame computing time : " << disp->duration.count() << endl;
		//cout << "Frame computing scale : " << disp->scale << endl;
		
		// Affichage de la fractale
		disp->dessin();
	}

	// Dezoome hors de la cible du clic
	static void clicDroit(SDL_Event& event, Affichage* disp)
	{
		// Calcul de la position du clic dans le plan complexe
		float x = disp->center.x + disp->scale*(((float)event.motion.x) / WIDTH - 0.5f);
		float y = disp->center.y + disp->scale*(((float)event.motion.y) / HEIGHT - 0.5f);

		// MAJ de la position du nouveau centre dans le plan complexe
		if (INTERACTIVE) {
			disp->center.x = x + (disp->center.x - x) / DEZOOM_FACTOR;
			disp->center.y = y + (disp->center.y - y) / DEZOOM_FACTOR;
			updateBigCenter(event, false);
		}


		// MAJ de l'echelle
		disp->scale /= DEZOOM_FACTOR;
		//BigFloat zoomFactor(true, 0, 0x80000000, 0, 0);
		BigFloat temp, temp2;
		BigFloat::mult(1/DEZOOM_FACTOR, *bigScale, temp);
		bigScale->reset();
		temp2.copy(*bigScale);
		BigFloat::add(temp, temp2, *bigScale);

		//Nouveau calcul de la fractale avec chrono
		disp->start = chrono::system_clock::now();

		if (GPU && BIG_FLOAT_SIZE == 0)
			affichageGPU(disp);
		else if (GPU)
			computeBigMandelGPU(disp, xCenter->pos, xCenter->decimals, yCenter->pos, yCenter->decimals, bigScale->decimals);
		else
			if (BIG_FLOAT_SIZE == 0)
				computeMandel(disp->pixels, disp->center, disp->scale);
			else {
				computeBigMandel(disp->pixels, *xCenter, *yCenter, *bigScale);
			}

			disp->end = chrono::system_clock::now();
			disp->duration = disp->end - disp->start;
			cout << "Frame computing time : " << disp->duration.count() << endl;
			//cout << "Frame computing scale : " << disp->scale << endl;

			// Affichage de la fractale
			disp->dessin();
	}
};

BigFloat* Events::bigScale = new BigFloat();
BigFloat* Events::xCenter = new BigFloat();
BigFloat* Events::yCenter = new BigFloat();