import { Component, MouseEvent } from 'react';
import { DraggableIcon, IPosition } from './draggable_icon';

interface IStaticIconProps {
  readonly iconClass: string;
  readonly label: string;
  readonly height: number;
  readonly width: number;
}

interface IActiveIconProps extends IStaticIconProps {
  readonly position: IPosition;
}

interface IComponentIconProps extends IStaticIconProps {
  readonly onMouseUp?: (e: MouseEvent) => any;
}

interface IComponentState {
  dragging: boolean;
  position: IPosition;
}

function createPosition(e: MouseEvent): IPosition {
  return { x : e.pageX, y : e.pageY };
}

function StaticIcon(props: IStaticIconProps) {
  return (
    <span
      className={props.iconClass}
      role="img"
      aria-label={props.label}
      style={{ width : props.width, height : props.height }}
    />
  );
}

function ActiveIcon(props: IActiveIconProps): JSX.Element {
  return (
    <div>
      <StaticIcon {...props}/>
      <DraggableIcon {...props}/>
    </div>
  );
}

export class ComponentIcon extends Component<IComponentIconProps, IComponentState> {
  constructor(props: IComponentIconProps) {
    super(props);
    this.setState({ dragging : false, position : { x : 0, y : 0 } });
  }

  public onMouseDown(e: MouseEvent) {
    this.setState({ dragging : true, position : createPosition(e) });
    e.stopPropagation();
    e.preventDefault();
  }

  public onMouseUp(e: MouseEvent) {
    this.setState({ dragging : false, position : createPosition(e) });
    e.stopPropagation();
    e.preventDefault();

    if (this.props.onMouseUp !== undefined) {
      this.props.onMouseUp(e);
    }
  }

  public render(): JSX.Element {
    if (this.state.dragging) {
      return <ActiveIcon position={this.state.position} {...this.props}/>;
    }
    return <StaticIcon {...this.props}/>;
  }
}
