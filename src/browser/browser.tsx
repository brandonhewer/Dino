
import { Component, MouseEvent } from 'react';
import { BrowserOptionMenu, IBrowserOptionMenuProps } from './browser_option_menu';

interface IBrowserProps {
  readonly optionIconClasses: string[];
  readonly optionIconLabels: string[];
  readonly optionWidth: number;
  readonly optionHeight: number;
  readonly windows: JSX.Element[];
}

interface IBrowserState {
  activeWindow: JSX.Element;
}

function createOptionProps(
  props: IBrowserProps,
  onClick: (i: number) => (e: MouseEvent) => any): IBrowserOptionMenuProps {
  return ({
    onClick,
    iconClasses: props.optionIconClasses,
    labels: props.optionIconLabels,
    optionHeight: props.optionHeight,
    optionWidth: props.optionWidth,
  });
}

export class Browser extends Component<IBrowserProps, IBrowserState> {
  private optionMenu: JSX.Element;

  constructor(props: IBrowserProps) {
    super(props);
    this.state = { activeWindow : this.props.windows[0] };

    const setWindow = (i: number) => (_: MouseEvent) => this.setActiveWindow(i);
    this.optionMenu = BrowserOptionMenu(createOptionProps(props, setWindow));
  }

  public setActiveWindow(index: number) {
    this.setState((_, props) => ({
      activeWindow : props.windows[index],
    }));
  }

  public render(): JSX.Element {
    return (
      <div id="browser">
        {this.optionMenu}
        {this.state.activeWindow}
      </div>
    );
  }
}
