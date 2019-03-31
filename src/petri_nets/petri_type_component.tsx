import { PureComponent } from 'react';
import { IPetriNet, PetriNetDiagram } from './petri_net_diagram';
import { PetriTypeForm } from './petri_type_form';
import { Selection } from './selection';

interface IPetriTypeComponentProps {
  readonly width: number;
  readonly height: number;
  readonly liveColour: string;
  readonly deadColour: string;
  readonly placeSize: number;
  readonly transitionSize: number;
  readonly selected: Selection;
  readonly cospan: string;
  readonly variable: string;
  readonly graph: IPetriNet;
  readonly transform: string;
  readonly setCospan: (x: string) => any;
  readonly setTransform: (x:string) => any;
  readonly setVariable: (x: string) => any;
  readonly setSVGReference: (x: SVGSVGElement) => void;
}

export class PetriTypeComponent extends PureComponent<IPetriTypeComponentProps, {}> {

  public render(): JSX.Element {
    const zIndex = 2 * this.props.selected;
    return (
      <div className='petrinet' z-index={zIndex}>
        <PetriTypeForm
          height={this.props.height * 0.4}
          width={this.props.width}
          cospan={this.props.cospan}
          variable={this.props.variable}
          transform={this.props.transform}
          selected={this.props.selected}
          setCospan={this.props.setCospan}
          setTransformation={this.props.setTransform}
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
