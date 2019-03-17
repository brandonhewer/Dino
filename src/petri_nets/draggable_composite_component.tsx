import { Component } from 'react';
import { DragSource } from 'react-dnd';
import { Selection } from './selection';
import { CompositePetriComponent } from './composite_petri_component';
import { IPetriNet } from '../petri_nets/petri_net_diagram';
import ItemTypes from './item_types';

interface ICompositeComponent {
  readonly liveColour: string;
  readonly deadColour: string;
  readonly placeSize: number;
  readonly transitionSize: number;
  readonly selected: Selection;
  readonly variable: string;
  readonly transform: string;
  readonly graph: IPetriNet;
  readonly setVariable: (x: string) => any;
  readonly setSVGReference: (x: SVGSVGElement) => void;
};

interface IDraggable {
  readonly connectDragSource?: any;
  readonly connectDragPreview?: any;
  readonly isDragging?: boolean;
};

interface IDraggableCompositeComponentProps extends ICompositeComponent, IDraggable {
  readonly index: number;
  readonly width: number;
  readonly height: number;
  readonly top: number;
  readonly left: number;
  readonly selected: Selection;
  readonly onClicked: (shift: boolean) => void;
}

const PetriTypeSource = {
  beginDrag(props: IDraggableCompositeComponentProps) {
    const { index, left, top } = props;
    return { index, left, top }
  },
};

class DraggableCompositeComponent extends Component<IDraggableCompositeComponentProps, {}> {

  render() {
    const { connectDragPreview, connectDragSource } = this.props;

    return connectDragPreview(
      <div
        className='petricomponent'
        style={{ 
          width:this.props.width, 
          height:this.props.height,
          top:this.props.top,
          left:this.props.left,
        }}
        onClick={(e) => this.props.onClicked(e.shiftKey)}
      >
        {connectDragSource(
          <div 
            className='handle' 
            style={{width: this.props.width}}
          />
        )}
        <CompositePetriComponent
          width={this.props.width}
          height={this.props.height}
          liveColour={this.props.liveColour}
          deadColour={this.props.deadColour}
          placeSize={this.props.placeSize}
          transitionSize={this.props.transitionSize}
          selected={this.props.selected}
          variable={this.props.variable}
          graph={this.props.graph}
          transform={this.props.transform}
          setVariable={this.props.setVariable}
          setSVGReference={this.props.setSVGReference}
        />
      </div>
    );
  }

}

const collect = (connect, monitor) => ({
  connectDragPreview: connect.dragPreview(),
  connectDragSource: connect.dragSource(),
  isDragging: monitor.isDragging()
});

const draggableNet = DragSource(ItemTypes.COMPOSITE_NET, PetriTypeSource, collect)(DraggableCompositeComponent);
export {draggableNet as PetriCompositeComponent};