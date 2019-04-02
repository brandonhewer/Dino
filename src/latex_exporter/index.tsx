import { LatexPetriNet } from './petrinet';
import { IPetriNet } from '../petri_nets';
import { h, render } from 'jsx-tikzcd';

export function createLatexFrom(graph: IPetriNet): string {
  return render(<LatexPetriNet graph={graph}/>);
}
