import { Component } from 'react';

interface IWorksheetComponentProps {
  element: JSX.Element;
  onClick: (e: MouseEvent) => any;
}

interface IWorksheetComponentState {
  isSelected: boolean;
}

export class WorksheetComponent
  extends Component<IWorksheetComponentProps, IWorksheetComponentState> {

  constructor(props: IWorksheetComponentProps) {
    super(props);
  }

  public onClick(e: MouseEvent) {
    this.setState({ isSelected : true });
    this.props.onClick(e);
  }

  public render(): JSX.Element {
    return (
      <div id={this.getSelector()} onClick={this.onClick}>
        {this.props.element}
      </div>
    );
  }

  private getSelector(): string {
    return `worksheet-component${this.state.isSelected ? '-selected' : ''}`;
  }
}
