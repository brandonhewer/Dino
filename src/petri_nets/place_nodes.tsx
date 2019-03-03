import * as d3 from 'd3';
import { Component } from 'react';
import { node_drag } from './node_drag';

export interface INode {
  readonly id: number;
}

export interface IPlaceNode extends INode {
  readonly type: number;
  readonly variance: number;
  readonly count: number;
}

interface IPlaceNodesProps {
  readonly nodes: IPlaceNode[][];
  readonly radius: number;
  readonly simulation: any;
}

export class PlaceNodes extends Component<IPlaceNodesProps, {}> {
  private ref: SVGGElement;

  public componentDidMount(): void {
    this.drawNodes();
  }

  public componentDidUpdate(): void {
    this.drawNodes();
  }

  public render(): JSX.Element {
    return <g className="places" ref={(ref: SVGGElement) => this.ref = ref} />;
  }

  private drawNodes(): void {
    const parts = d3.select(this.ref)
      .selectAll('g.nodes')
      .data(this.props.nodes);

    parts.enter().append('g').attr('class', 'nodes');
    parts.exit().remove();

    const nodes = d3.select(this.ref)
      .selectAll('g.nodes')
      .data(this.props.nodes)
      .selectAll('circle.node')
      .data(d => d);

    nodes.attr('class', 'node')
      .attr('r', this.props.radius)
      .style('fill', d => d.variance == 0 ? 'white' : 'lightgray')
      .style('stroke', '#000')
      .style('stroke-width', '2')
      .call(node_drag(this.props.simulation));

    nodes.enter().append('circle')
      .attr('class', 'node')
      .attr('r', this.props.radius)
      .style('fill', d => d.variance == 0 ? 'white' : 'lightgray')
      .style('stroke', '#000')
      .style('stroke-width', '2')
      .call(node_drag(this.props.simulation));

    nodes.exit().remove();

    const text = d3.select(this.ref)
      .selectAll('g.nodes')
      .data(this.props.nodes)
      .selectAll('text')
      .data(d => d);

    text.text(d => d.count > 0 ? 'f' : '')
      .attr('class', 'function')
      .call(node_drag(this.props.simulation));

    text.enter().append('text')
      .attr('class', 'function')
      .text(d => d.count > 0 ? 'f' : '')
      .call(node_drag(this.props.simulation));

    text.exit().remove();
  }

}
