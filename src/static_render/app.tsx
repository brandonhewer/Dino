import * as React from 'react';
import HTML5Backend from 'react-dnd-html5-backend';
import { DragDropContext } from 'react-dnd';
import Worksheet from './worksheet'

const addPetriNetHotKeys = ['command+n', 'ctrl+n'];
const removePetriNetHotKeys = ['del', 'command+d'];
const composePetriNetHotKeys = ['command+.', 'ctrl+.'];
const savePetriNetHotKeys = ['command+s', 'ctrl+s'];

function App() {
  return (
    <Worksheet
      componentHeight={400}
      componentWidth={400}
      addPetriNetHotKeys={addPetriNetHotKeys}
      removePetriNetHotKeys={removePetriNetHotKeys}
      composePetriNetHotKeys={composePetriNetHotKeys}
      savePetriNetHotKeys={savePetriNetHotKeys}
    />
  );
}

export default DragDropContext(HTML5Backend)(App);
