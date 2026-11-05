// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the sirens per vehicle, the SirenSettings part's second job. The game lights siren1 to siren20 of a vehicle, so a
// vehicle made for 32 sirens keeps the rest dark. SIRENS_PER_VEHICLE above 20 lights them by a second call of the game's own
// renderer, every structure of the game staying at 20 (D23). With DIAGNOSTIC_MODE=1, measured first and still, the module counts
// the vehicle models whose sirens the game draws and logs which of them carry siren bones past siren20, which the second call
// lights and which lose siren bones past bone 127 (SPECIFICATIONS 11.16.2).
#pragma once

namespace TweaksV
{
	// Finds and checks the siren count's sites and starts its measurement, which redirects the vehicle lights'
	// call of the siren lights' renderer. Called by the table of the parts measured first, with
	// DIAGNOSTIC_MODE=1 only.
	void MeasureSirenCount();

	// The sirens per vehicle (D23, D38), started by the SirenSettings part's entry of the feature table, after InitSirenIds:
	// SIRENS_PER_VEHICLE above 20 asks D40's check and the light stores, then both halves find and check their sites, and all
	// of it is one write.
	void InitSirensPerVehicle();
}
