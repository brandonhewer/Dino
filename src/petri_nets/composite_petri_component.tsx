import { Component } from 'react';
import { Selection } from './selection';
import { CompositePetriForm } from './composite_petri_form';
import { IPetriNet, PetriNetDiagram } from './petri_net_diagram';

interface ICompositePetriComponentProps {
  readonly width: number;
  readonly height: number;
  readonly liveColour: string;
  readonly deadColour: string;
  readonly placeSize: number;
  readonly transitionSize: number;
  readonly selected: Selection;
  readonly variable: string;
  readonly graph: IPetriNet;
  readonly transform: string;
  readonly setVariable: (x: string) => any;
  readonly setSVGReference: (x: SVGSVGElement) => void;
}

export class CompositePetriComponent extends Component<ICompositePetriComponentProps, {}> {

  public render(): JSX.Element {
    const zIndex = 2 * this.props.selected;
    return (
      <div className='petrinet' z-index={zIndex}>
        <CompositePetriForm
          height={this.props.height * 0.4}
          width={this.props.width}
          variable={this.props.variable}
          transform={this.props.transform}
          selected={this.props.selected}
          setVariable={this.props.setVariable}
        />
        <PetriNetDiagram
          graphData={this.props.graph}
          width={this.props.width}
          height={this.props.height * 0.6}
          liveColour={'#666'}
          deadColour={'#333'}
          placeSize={18}
          transitionSize={28}
          setSVGReference={this.props.setSVGReference}
          zIndex={zIndex - 1}
        />
      </div>
    );
  }
}
