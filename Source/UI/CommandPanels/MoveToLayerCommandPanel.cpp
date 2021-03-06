/*
    This file is part of Helio Workstation.

    Helio is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Helio is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Helio. If not, see <http://www.gnu.org/licenses/>.
*/

#include "Common.h"
#include "MoveToLayerCommandPanel.h"
#include "ProjectTreeItem.h"
#include "MidiRollToolbox.h"
#include "PianoLayerTreeItem.h"
#include "PianoRoll.h"
#include "Icons.h"
#include "TriggersTrackMap.h"
#include "MainLayout.h"
#include "AudioCore.h"
#include "Instrument.h"
#include "MidiLayer.h"
#include "InternalClipboard.h"
#include "App.h"
#include "CommandIDs.h"

//===----------------------------------------------------------------------===//
// Move-to-layer command panel
//

MoveToLayerCommandPanel::MoveToLayerCommandPanel(PianoRoll &targetRoll,
                                                 ProjectTreeItem &parentProject) :
    roll(targetRoll),
    project(parentProject)
{
    this->initLayersPanel(false);
}

MoveToLayerCommandPanel::~MoveToLayerCommandPanel()
{
}

void MoveToLayerCommandPanel::handleCommandMessage(int commandId)
{
    const Array<PianoLayerTreeItem *> &layerItems =
        this->project.findChildrenOfType<PianoLayerTreeItem>();
    
    if (commandId >= CommandIDs::MoveEventsToLayer &&
        commandId <= (CommandIDs::MoveEventsToLayer + layerItems.size()))
    {
        const int layerIndex = commandId - CommandIDs::MoveEventsToLayer;
        
        const MidiEventSelection::MultiLayerMap &selections = this->roll.getLassoSelection().getMultiLayerSelections();
        const int numSelected = this->roll.getLassoSelection().getNumSelected();
        const MidiLayer *layerOfFirstSelected = (numSelected > 0) ? (this->roll.getLassoSelection().getSelectedItem(0)->getEvent().getLayer()) : nullptr;
        const bool hasMultiLayerSelection = (selections.size() > 1);
        const bool alreadyBelongsTo = hasMultiLayerSelection ? false : (layerItems[layerIndex]->getLayer() == layerOfFirstSelected);

        if (! alreadyBelongsTo)
        {
            //Logger::writeToLog("Moving notes to " + layerItems[layerIndex]->getXPath());
            MidiRollToolbox::moveToLayer(this->roll.getLassoSelection(), layerItems[layerIndex]->getLayer());
            layerItems[layerIndex]->setSelected(false, false, sendNotification);
            layerItems[layerIndex]->setSelected(true, true, sendNotification);
            this->dismiss();
        }

        return;
    }
}

void MoveToLayerCommandPanel::initLayersPanel(bool shouldAddBackButton)
{
    ReferenceCountedArray<CommandItem> cmds;
    
    if (shouldAddBackButton)
    {
        cmds.add(CommandItem::withParams(Icons::left, CommandIDs::Back, TRANS("menu::back")));
    }
    
    const Array<PianoLayerTreeItem *> &layers =
        this->project.findChildrenOfType<PianoLayerTreeItem>();
    
    for (int i = 0; i < layers.size(); ++i)
    {
        const MidiEventSelection::MultiLayerMap &selections = this->roll.getLassoSelection().getMultiLayerSelections();
        const int numSelected = this->roll.getLassoSelection().getNumSelected();
        const MidiLayer *layerOfFirstSelected = (numSelected > 0) ? (this->roll.getLassoSelection().getSelectedItem(0)->getEvent().getLayer()) : nullptr;
        const bool hasMultiLayerSelection = (selections.size() > 1);
        const bool belongsTo = hasMultiLayerSelection ? false : (layers.getUnchecked(i)->getLayer() == layerOfFirstSelected);
        
        const String name(layers.getUnchecked(i)->getXPath());
        const Colour colour(layers.getUnchecked(i)->getColour());
        cmds.add(CommandItem::withParams(belongsTo ? Icons::apply : Icons::layer, CommandIDs::MoveEventsToLayer + i, name)->colouredWith(colour));
    }
    
    this->updateContent(cmds, CommandPanel::SlideLeft);
}

void MoveToLayerCommandPanel::dismiss()
{
    this->roll.grabKeyboardFocus();
    this->getParentComponent()->exitModalState(0);
}
