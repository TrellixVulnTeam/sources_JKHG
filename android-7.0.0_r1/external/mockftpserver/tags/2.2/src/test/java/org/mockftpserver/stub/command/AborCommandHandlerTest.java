/*
 * Copyright 2007 the original author or authors.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package org.mockftpserver.stub.command;

import org.mockftpserver.core.command.*;
import org.mockftpserver.core.command.AbstractCommandHandlerTestCase;

/**
 * Tests for the AborCommandHandler class
 * 
 * @version $Revision$ - $Date$
 * 
 * @author Chris Mair
 */
public final class AborCommandHandlerTest extends AbstractCommandHandlerTestCase {

    private AborCommandHandler commandHandler;

    /**
     * Test the handleCommand() method
     */
    public void testHandleCommand() throws Exception {
        final Command COMMAND = new Command(CommandNames.ABOR, EMPTY);

        session.sendReply(ReplyCodes.ABOR_OK, replyTextFor(ReplyCodes.ABOR_OK));
        replay(session);

        commandHandler.handleCommand(COMMAND, session);
        verify(session);
        
        verifyNumberOfInvocations(commandHandler, 1);
        verifyNoDataElements(commandHandler.getInvocation(0));
    }

    /**
     * Perform initialization before each test
     * 
     * @see org.mockftpserver.core.command.AbstractCommandHandlerTestCase#setUp()
     */
    protected void setUp() throws Exception {
        super.setUp();
        commandHandler = new AborCommandHandler();
        commandHandler.setReplyTextBundle(replyTextBundle);
    }

}
