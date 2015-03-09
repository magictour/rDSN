# pragma once

# include <rdsn/internal/task.h>
# include <rdsn/internal/network.h>
# include <rdsn/internal/synchronize.h>

namespace rdsn {

    class rpc_engine;
    class rpc_client_matcher;
    
    class network
    {
    public:
        template <typename T> static network* create(rpc_engine* srv, network* inner_provider)
        {
            return new T(srv, inner_provider);
        }

    public:
        network(rpc_engine* srv, network* inner_provider); 
        virtual ~network() {}

        rpc_engine* engine() const { return _engine;  }
        std::shared_ptr<rpc_client_matcher> new_client_matcher();
        void call(message_ptr& request, rpc_response_task_ptr& call);

        rpc_server_session_ptr get_server_session(const end_point& ep);
        void on_server_session_accepted(rpc_server_session_ptr& s);
        void on_server_session_disconnected(rpc_server_session_ptr& s);

        rpc_client_session_ptr get_client_session(const end_point& ep);
        void on_client_session_disconnected(rpc_client_session_ptr& s);

        virtual error_code start(int port, bool client_only) = 0;
        virtual const end_point& address() = 0;
        virtual rpc_client_session_ptr create_client_session(const end_point& server_addr) = 0;

    protected:
        rpc_engine *_engine;
        
        typedef std::map<end_point, rpc_client_session_ptr> client_sessions;
        client_sessions               _clients;
        utils::rw_lock                _clients_lock;

        typedef std::map<end_point, rpc_server_session_ptr> server_sessions;
        server_sessions               _servers;
        utils::rw_lock                _servers_lock;

    public:
        static int max_faked_port_for_client_only_node;
    };


    class rpc_client_session : public ref_object
    {
    public:
        rpc_client_session(network& net, const end_point& remote_addr, std::shared_ptr<rpc_client_matcher>& matcher);
        bool on_recv_reply(uint64_t key, message_ptr& reply, int delay_handling_milliseconds = 0);
        void on_disconnected();
        void call(message_ptr& request, rpc_response_task_ptr& call);
        const end_point& remote_address() const { return _remote_addr; }

        virtual void connect() = 0;
        virtual void send(message_ptr& msg) = 0;

    protected:
        network                  &_net;
        end_point                 _remote_addr;
        std::shared_ptr<rpc_client_matcher> _matcher;
    };

    DEFINE_REF_OBJECT(rpc_client_session)

    class rpc_server_session : public ref_object
    {
    public:
        rpc_server_session(network& net, const end_point& remote_addr);
        void on_recv_request(message_ptr& msg, int delay_handling_milliseconds = 0);
        void on_disconnected();
        const end_point& remote_address() const { return _remote_addr; }

        virtual void send(message_ptr& reply_msg) = 0;

    protected:
        network&   _net;
        end_point _remote_addr;
    };

    DEFINE_REF_OBJECT(rpc_server_session)
}
