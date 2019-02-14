import * as React from 'react';
import * as ReactDOM from 'react-dom';
import { PetriTypeComponent } from '../petri_nets';

/*
const bindings = require('bindings');
const naturality = bindings('Naturality.node');

const transform = '(a -> a) -> a => (a -> a) -> a';
const type = '(1 -> 0) -> 0 => (1 -> 0) -> 0';

const transformation = naturality.create_transformation(transform);
transformation.set_cospan(type);
*/

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
