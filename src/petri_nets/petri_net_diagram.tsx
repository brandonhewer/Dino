import * as d3 from 'd3';
import { Component } from 'react';
import { Edges, IEdge, IEdges } from './edges';
import { INode, IPlaceNode, PlaceNodes } from './place_nodes';
import { TransitionNodes } from './transition_nodes';

export interface IPetriNet {
  readonly nodes: IPlaceNode[][];
  readonly transitions: INode[];
  readonly edges: IEdges;
}

interface IPetriNetProps {
  readonly graphData: IPetriNet;
  readonly width: number;
  readonly height: number;
  readonly liveColour: string;
  readonly deadColour: string;
  readonly placeSize: number;
  readonly transitionSize: number;
}

function all(xs: boolean[]): boolean {
  return xs.reduce((x, y) => x && y, true);
}

function incoming(to: number, edges: IEdge[]): IEdge[] {
  return edges.filter(edge => edge.target == to);
}

function activeEdge(edge: IEdge, nodes: IPlaceNode[][]): boolean {
  return false;
}

function activeNode(node: INode, edges: IEdge[], nodes: IPlaceNode[][]): boolean {
  return all(incoming(node.id, edges).map(edge => activeEdge(edge, nodes)));
}

function active(node: INode, graph: IPetriNet): boolean {
  return activeNode(node, graph.edges.incoming, graph.nodes);
}

function createSimulation(nodes: INode[], edges: IEdges, centre: [number, number]): any {
  const links = edges.incoming.concat(
    edges.outgoing,
    edges.invisible,
  );

  const simulation: any = d3.forceSimulation()
    .force('link', d3.forceLink().distance(
      (d: IEdge) => d.distance * 30,
    ))
    .force('charge', d3.forceManyBody().strength(-300))
    .force('center', d3.forceCenter(...centre))
    .nodes(nodes);

  simulation.force('link').links(links);
  return simulation;
}

interface IPetriNetDiagramProps {
  nodes: INode[];
  simulation: any;
}

export class PetriNetDiagram extends Component<IPetriNetProps, IPetriNetDiagramProps> {

  public static getDerivedStateFromProps(nextProps, prevState) {
    const nodes = nextProps.graphData.transitions.concat(...nextProps.graphData.nodes);
    prevState.simulation.stop();
    return {nodes, simulation: createSimulation(nodes, nextProps.graphData.edges, [
      nextProps.width / 2, nextProps.height / 2,
    ])};
  }

  constructor(props: IPetriNetProps) {
    super(props);

    const nodes = props.graphData.transitions.concat(...props.graphData.nodes);

    this.state = {
      nodes,
      simulation: createSimulation(nodes, props.graphData.edges, [
        this.props.width / 2, this.props.height / 2,
      ]),
    };
  }

  public componentDidMount(): void {
    this.runSimulation(this.state.simulation);
  }

  public componentDidUpdate(): void {
    this.runSimulation(this.state.simulation);
  }

  public render(): JSX.Element {
    return (
      <svg className="petrinet" height={this.props.height} width={this.props.width}>
        <defs>
          <marker
            id="arrow"
            viewBox="0 0 10 10"
            refX="5"
            refY="5"
            markerWidth="6"
            markerHeight="6"
            orient="auto-start-reverse"
          >
            <path d="M 0 0 L 10 5 L 0 10 z" />
          </marker>
        </defs>
        <Edges
          incoming={this.props.graphData.edges.incoming}
          outgoing={this.props.graphData.edges.outgoing}
          invisible={this.props.graphData.edges.invisible}
        />
        <PlaceNodes
          nodes={this.props.graphData.nodes}
          radius={this.props.placeSize}
          simulation={this.state.simulation}
        />
        <TransitionNodes
          transitions={this.props.graphData.transitions}
          width={this.props.transitionSize}
          height={this.props.transitionSize}
          simulation={this.state.simulation}
          fillColour={(d: INode) => active(d, this.props.graphData) ? this.props.liveColour : this.props.deadColour}
          onClick={function (node, index) {}}
        />
      </svg>
    );
  }

  private runSimulation(simulation: any): void {
    const place = d3.select('.places')
      .selectAll('.nodes').selectAll('circle.node');

    const transition = d3.select('.transitions').selectAll('rect.node');
    const edge = d3.select('.edges').selectAll('polyline.link');
    const invisibleEdge = d3.select('.edges').selectAll('line.link');

    simulation.nodes(this.state.nodes).on('tick', () => {
      edge.attr('points', (d: any) => {
        const sx = d.source.x;
        const sy = d.source.y;
        const ex = d.target.x;
        const ey = d.target.y;
        const mx = (sx + ex) / 2;
        const my = (sy + ey) / 2;
        return `${sx},${sy} ${mx},${my} ${ex},${ey}`;
      });
      invisibleEdge
        .attr('x1', (d: any) => d.source.x)
        .attr('y1', (d: any) => d.source.x)
        .attr('x2', (d: any) => d.target.x)
        .attr('y2', (d: any) => d.target.x);
      place
        .attr('cx', (d: any) => d.x)
        .attr('cy', (d: any) => d.y);
      transition
        .attr('x', (d: any) => d.x - this.props.transitionSize / 2)
        .attr('y', (d: any) => d.y - this.props.transitionSize / 2);
    });
  }
}
