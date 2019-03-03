import { Component } from 'react';
import { IPetriNet, PetriNetDiagram } from './petri_net_diagram';

import RGL, { WidthProvider } from 'react-grid-layout';

const ReactGridLayout = WidthProvider(RGL);

const bindings = require('bindings');
const naturality = bindings('Naturality.node');

interface ITransformation {
  readonly graph: (x: string) => IPetriNet;
  readonly set_cospan: (x: string) => ITransformation;
  readonly cospan_string: () => string;
}

interface IPetriTypeComponentProps {
  readonly width: number;
  readonly height: number;
  readonly liveColour: string;
  readonly deadColour: string;
  readonly placeSize: number;
  readonly transitionSize: number;
}

interface IPetriTypeComponentState {
  transformation: ITransformation;
  graph: IPetriNet;
  cospan: string;
  variable: string;
}

const formLayout = [
  { i:'transform', x:0, y:0, w:3, h:1 },
  { i:'cospan', x:0, y:1, w:3, h:1 },
  { i:'variable', x:3, y:0, w:1, h:2 },
];

interface IPetriTypeFormProps {
  readonly cospan: string;
  readonly variable: string;
  readonly width: number;
  readonly rowHeight: number;
  readonly setTransformation: (x: string) => void;
  readonly setCospan: (x: string) => void;
  readonly setVariable: (x: string) => void;
}

function PetriTypeForm(props: IPetriTypeFormProps) {
  return (
    <form className="form">
      <ReactGridLayout
        className="layout"
        layout={formLayout}
        cols={4}
        rowHeight={props.rowHeight}
        width={props.width}
        isDraggable={false}
        containerPadding={[0, 0]}
        marging={[0, 0]}
      >
        <div key="transform">
          <textarea
            style={{ height:'100%', width:'100%' }}
            onChange={(e: any) => props.setTransformation(e.target.value)}
          />
        </div>
        <div key="cospan">
          <textarea
            style={{ height:'100%', width:'100%' }}
            value={props.cospan}
            onChange={(e: any) => props.setCospan(e.target.value)}
          />
        </div>
        <div key="variable">
          <textarea
            style={{ height:'100%', width:'100%', right:'100px' }}
            value={props.variable}
            onChange={(e: any) => props.setVariable(e.target.value)}
          />
        </div>
      </ReactGridLayout>
    </form>
  );
}

export class PetriTypeComponent
  extends Component<IPetriTypeComponentProps, IPetriTypeComponentState> {
  private readonly layout: any;

  constructor(props: IPetriTypeComponentProps) {
    super(props);

    this.state = {
      graph: {
        nodes: [],
        transitions: [],
        edges: {
          incoming:[],
          outgoing:[],
          invisible:[],
        },
      },
      cospan: '',
      variable: '',
      transformation: undefined,
    };

    this.layout = [
      { i:'form', x:0, y:0, w:1, h:1 },
      { i:'diagram', x:0, y:1, w:1, h:5 },
    ];
  }

  public render(): JSX.Element {
    return (
        <div key="form">
          <PetriTypeForm
            rowHeight={this.props.height / 6}
            width={this.props.width}
            cospan={this.state.cospan}
            variable={this.state.variable}
            setCospan={this.setCospan.bind(this)}
            setTransformation={this.setTransformation.bind(this)}
            setVariable={this.setVariable.bind(this)}
          />
          <PetriNetDiagram
            graphData={this.state.graph}
            width={this.props.width}
            height={this.props.height * (5 / 6)}
            liveColour={'#666'}
            deadColour={'#333'}
            placeSize={18}
            transitionSize={28}
          />
        </div>
    );
  }

  private setGraph(transformation: ITransformation, variable: string) {
    if (variable.length > 0) {
      try {
        this.setState({
          graph: transformation.graph(variable),
        });
      } catch (err) {}
    }
  }

  private setTransformation(transform: string) {
    const transformation = naturality.create_transformation(transform);

    this.setState({
      transformation,
      cospan: transformation.cospan_string(),
    });

    this.setGraph(transformation, this.state.variable);
  }

  private setCospan(cospan: string) {
    this.setState({ cospan });

    try {
      this.state.transformation.set_cospan(cospan);
    } catch (err) {
      return;
    }

    this.setGraph(this.state.transformation, this.state.variable);
  }

  private setVariable(variable: string) {
    this.setState({ variable });
    this.setGraph(this.state.transformation, variable);
  }
}
