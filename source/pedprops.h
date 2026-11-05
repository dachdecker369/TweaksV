// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the PedProps part: the props per anchor of a ped model. A ped model's props - hats, glasses, watches -
// hang on 13 anchors and come in variation files, its own and one per clothing pack; the game counts an anchor's
// props over all of them, but its checks take that total, and a worn prop's index, as 8 bits, so from 256 props on
// one anchor it refuses the props past (total mod 256). The part widens those eleven narrowings to 16 bits, so that
// an anchor holds up to 65535 props in all; with DIAGNOSTIC_MODE=1 it also logs how many props each ped model has
// on each anchor. It ships off: the raise is experimental, untested in the game past 255 (ISSUES.md, T-PEDPROPS).
#pragma once

namespace TweaksV
{
	// The PedProps part (SPECIFICATIONS 11.11), started by its entry of the feature table, from DllMain, while
	// [PEDPROPS] is on: finds and checks the eleven narrowings, the calls around them and the complete sets, widens
	// the eleven in one write and, with DIAGNOSTIC_MODE=1, starts the measurement of the props per anchor.
	void InitPedProps();

	// The ped props' measurement alone while [PEDPROPS] is off, from the table of the parts measured first, so that a
	// load shows whether it needs the part: DIAGNOSTIC_MODE=1 only, and it writes nothing. With the part on, nothing:
	// InitPedProps measures.
	void MeasurePedPropsWhileOff();
}
