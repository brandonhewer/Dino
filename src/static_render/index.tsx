import * as React from 'react';
import * as ReactDOM from 'react-dom';
import { PetriTypeComponent } from '../petri_nets';

const element = (
  <PetriTypeComponent
    width={400}
    height={400}
    liveColour={'#666'}
    deadColour={'#333'}
    placeSize={18}
    transitionSize={28}
  />
);

ReactDOM.render(element, document.getElementById('root'));
