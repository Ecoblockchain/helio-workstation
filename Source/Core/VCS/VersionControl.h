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

#pragma once

class ProjectInfo;
class VersionControlEditor;

#include "TreeItem.h"
#include "TrackedItemsSource.h"
#include "SafeTreeItemPointer.h"

#include "Serializable.h"
#include "ProjectListener.h"

#include "Delta.h"
#include "Revision.h"
#include "Head.h"
#include "Pack.h"
#include "Client.h"
#include "StashesRepository.h"

#include "Key.h"

class VersionControl :
    public Serializable,
    public ChangeListener,
    public ChangeBroadcaster
{
public:

    explicit VersionControl(WeakReference<VCS::TrackedItemsSource> parent,
                   const String &existingId = "",
                   const String &existingKeyBase64 = "");

    ~VersionControl() override;


    //===------------------------------------------------------------------===//
    // Push-pull stuff
    //===------------------------------------------------------------------===//

    VCS::Client *getRemote()
    { return this->remote; }

    inline String getParentName() const
    { return this->parentItem->getVCSName(); }

    inline String getPublicId() const
    { return this->publicId; }

    inline MemoryBlock getKey()
    { return this->key.getKeyData(); }

    inline int64 getVersion() const
    { return this->historyMergeVersion; }

    inline void incrementVersion()
    { this->historyMergeVersion += 1; }

    MD5 calculateHash() const;

    void mergeWith(VersionControl &remoteHistory);


    //===------------------------------------------------------------------===//
    // VCS
    //===------------------------------------------------------------------===//

    VersionControlEditor *createEditor();

    VCS::Head &getHead() { return this->head; }

    VCS::Revision getRoot() { return this->root; }


    void moveHead(const VCS::Revision revision);

    void checkout(const VCS::Revision revision);

    void cherryPick(const VCS::Revision revision, const Array<Uuid> uuids);


    bool resetChanges(SparseSet<int> selectedItems);

    bool resetAllChanges();

    bool commit(SparseSet<int> selectedItems, const String &message);

    void quickAmendItem(VCS::TrackedItem *targetItem); // for projectinfo


    bool stash(SparseSet<int> selectedItems, const String &message, bool shouldKeepChanges = false);

    bool applyStash(const VCS::Revision stash, bool shouldKeepStash = false);

    bool applyStash(const String &stashId, bool shouldKeepStash = false);

    
    bool hasQuickStash() const;
    
    bool quickStashAll();

    bool applyQuickStash();


    
    //===------------------------------------------------------------------===//
    // Serializable
    //===------------------------------------------------------------------===//

    XmlElement *serialize() const override;

    void deserialize(const XmlElement &xml) override;

    void reset() override;


    //===------------------------------------------------------------------===//
    // ChangeListener
    //===------------------------------------------------------------------===//

    void changeListenerCallback(ChangeBroadcaster* source) override;
    
protected:

    StringArray recursiveGetHashes(const VCS::Revision revision) const;

    void recursiveTreeMerge(VCS::Revision localRevision, VCS::Revision remoteRevision);

    VCS::Revision getRevisionById(const VCS::Revision startFrom, const String &id) const;

    VCS::Pack::Ptr pack;

    VCS::StashesRepository::Ptr stashes;

    VCS::Head head;

    // само дерево vcs
    VCS::Revision root;

    ScopedPointer<VCS::Client> remote;

    WeakReference<VCS::TrackedItemsSource> parentItem;

protected:

    String publicId;

    VCS::Key key;

    int64 historyMergeVersion;

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VersionControl)

};
